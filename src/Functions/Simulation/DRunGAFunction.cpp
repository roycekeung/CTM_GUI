#include "DRunGAFunction.h"

#include "PanelWidgets/Simulation/DGASettingsPanel.h"
#include "DDockWidgetHandler.h"
#include "DTabToolBar.h"
#include "DRunManager.h"
#include "DFunctionHandler.h"
#include "GraphicsItems/DGraphicsScene.h"
#include "Functions/DBaseNaviFunction.h"
#include "Functions/Simulation/DPlayBackFunction.h"
#include "Utils/DDefaultParamData.h"

#include "Sim_Builder.h"
#include "GA_Module.h"
#include "I_SigRecord.h"

#include "SigDataConvertor_Core.h"
#include "Runner_CoreMultiThread.h"
#include "RunCase_Core.h"

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DRunGAFunction::DRunGAFunction() : QObject(nullptr), I_Function(){
}

DRunGAFunction::~DRunGAFunction() {
	delete this->m_GAModule;
}

void DRunGAFunction::initFunctionHandler() {
	//setup the panel widget
	this->m_panelWidget = new DGASettingsPanel(genSigString(), genDmdString());
	this->ref_dockWidgetHandler->setRightDockWidget(this->m_panelWidget, "Run Optimisation");
	this->m_panelWidget->setupButtons(this);
	this->showDefaultSettings();

	//set up the graphics scene
	this->ref_graphicsScene->clearSelection();

	//disable other stuff
	this->ref_baseNaviFunction->setIgnoreEvents(true);
	this->ref_tabToolBar->setEnabled(false);

	//make GA module
	this->m_GAModule = new DISCO2_GA::GA_Module{};
	this->ref_sigConvertor = new DISCO2_GA_coreConvertor::SigDataConvertor_Core{ *(this->ref_scn), -1 };
	this->m_GAModule->setSigDataConvertor(this->ref_sigConvertor);
	this->m_GAModule->setRunCaseFactory(new DISCO2_GA_coreConvertor::RunCaseFactory_Core{});
	this->ref_runner = new DISCO2_GA_coreConvertor::Runner_CoreMultiThread{ 1, *(this->ref_scn) };
	this->m_GAModule->setRunner(this->ref_runner);
}

// --- --- --- --- --- Checking and creation Functions --- --- --- --- ---

void DRunGAFunction::checkAndRunGA() {
	if (this->m_panelWidget->getSigSetInput().isEmpty()) {
		this->m_panelWidget->failMessage("No signal set input selected");
		return;
	}
	if (this->m_panelWidget->getDmdSetInput().isEmpty()) {
		this->m_panelWidget->failMessage("No demand set input selected");
		return;
	}

	int sigSet = -1;
	int dmdSet = -1;
	int t_sigSet = -1;

	try {
		sigSet = this->m_sigSetStrings.at(this->m_panelWidget->getSigSetInput());
		dmdSet = this->m_dmdSetStrings.at(this->m_panelWidget->getDmdSetInput());

		this->ref_scn->get()->getSimBuilder()
			.useSigSet(sigSet)
			.useDmdSet(dmdSet)
			.setSimEndTime((int64_t)this->m_panelWidget->getTimeInput() * 1000)
			.setStoreRecType(DISCO2_API::Sim_Builder::Type_Record::minimal)
			.build();
		
		//Setup sig convertor
		this->ref_sigConvertor->setSigSetUsing(sigSet);
		this->ref_runner->setNumOfThreads(this->m_panelWidget->getNumOfThreads());

		//Setup GA
		this->m_GAModule->initSigDataConvertor();
		this->m_GAModule->set_use_initial_seed(true);

		this->m_GAModule->set_Populcation_size(this->m_panelWidget->getPopSize());
		this->m_GAModule->set_no_generation(this->m_panelWidget->getNumOfGen());
		this->m_GAModule->set_no_sub_generation(this->m_panelWidget->getNumOfSubGen());
		this->m_GAModule->set_Elite_rate(this->m_panelWidget->getEliteRate());
		this->m_GAModule->set_crossover_multiplier(this->m_panelWidget->getCrossoverRate());
		this->m_GAModule->set_Mutation_rate(this->m_panelWidget->getMutationRate());
		this->m_GAModule->set_Power_factor(this->m_panelWidget->getPowerFactor());

		std::vector<std::set<int>> unfreezeList;
		std::vector<int> intersecGp;

		for (auto& sigId : this->ref_scn->get()->getSignal().getSigSet(sigSet).getSigIds()) {
			unfreezeList.push_back({ sigId });
			intersecGp.push_back(sigId);

			this->m_GAModule->set_CycleTime_Changeable(sigId, (this->m_panelWidget->getGAType() && this->m_panelWidget->getMatchCyc()) ? false : this->m_panelWidget->getOptCyc());

			this->m_GAModule->set_Offset_Changeable(sigId, this->m_panelWidget->getOptOffset());
			for (auto& thing : this->m_GAModule->intersecs_inf.at(sigId).phase_duration_inf_collections)
				thing.phase_changeable_by_value = this->m_panelWidget->getOptGreen();
		}

		if(this->m_panelWidget->getMatchCyc())
			this->m_GAModule->set_intersections_gp({ intersecGp });

		if (this->m_panelWidget->getGAType()) {
			this->m_GAModule->set_unfreeze_list(unfreezeList);
			this->m_GAModule->set_sga_use_cyc_subgen(this->m_panelWidget->getOptCyc());

			this->m_GAModule->Run_SGA();
		}
		else {
			this->m_GAModule->Run_NGA();
		}

		t_sigSet = this->ref_scn->get()->getSigEditor().sigSet_clone(sigSet);
		this->ref_sigConvertor->editCase(this->m_GAModule->getBestChromosome(), *(this->ref_scn), t_sigSet);
		this->ref_scn->get()->getSimBuilder()
			.useSigSet(t_sigSet)
			.setStoreRecType(DISCO2_API::Sim_Builder::Type_Record::all)
			.build();
		this->ref_scn->get()->runSim();
	}
	catch (std::exception& e) {
		//print fail msg in the widget
		this->m_panelWidget->failMessage({ e.what() });
		//early exit as operation failed
		return;
	}

	//store the results
	//VisInfo
	DISCO2_API::VisInfo tempVisInfo{};
	tempVisInfo.setName(this->m_panelWidget->getNameInput().toStdString());
	tempVisInfo.setDetails(this->m_panelWidget->getDetailsInput().toStdString());
	tempVisInfo.addSecret("SigSet", "Optimised from " + std::to_string(sigSet));
	tempVisInfo.addSecret("DmdSet", std::to_string(dmdSet));
	tempVisInfo.addSecret("Time", std::to_string(this->m_panelWidget->getTimeInput()));

	//sigRec
	std::unordered_map<int, std::unordered_map<int, std::pair<std::vector<int64_t>, std::vector<int64_t>>>> tSigRec;
	for (auto sigId : this->ref_scn->get()->getSignal().getSigSet(t_sigSet).getSigIds()) {
		const DISCO2_API::I_SigRecord* sigRec = dynamic_cast<const DISCO2_API::I_SigRecord*>(this->ref_scn->get()->getSignal().getSigSet(t_sigSet).getSigCtrl(sigId));
		if (sigRec)
			tSigRec.emplace(sigId, sigRec->getAllOnOffTimes());
	}

	this->m_savedIndex = this->ref_runManager->addRec(
		this->ref_scn->get()->getRecHandler()->deepClone(),
		std::move(tSigRec),
		std::move(tempVisInfo));
	this->ref_scn->get()->getSigEditor().sigSet_delete(t_sigSet);

	//display the results
	auto* rec = this->ref_runManager->getRec(this->m_savedIndex);
	this->m_panelWidget->showResults(true,
		rec->getTotalDelay() / 1000,
		rec->getRealisedDemand(),
		rec->getAvgDelay() / 1000);
}

void DRunGAFunction::saveSigSet() {
	int newSigSetId = -1;
	try {
		newSigSetId = this->ref_scn->get()->getSigEditor().sigSet_clone(this->ref_sigConvertor->getSigSetUsing());
		this->ref_sigConvertor->editCase(this->m_GAModule->getBestChromosome(), *(this->ref_scn), newSigSetId);

		//store the visInfo
		DISCO2_API::VisInfo tempVisInfo{};
		tempVisInfo.setName(this->m_panelWidget->getSigNameInput().toStdString());
		tempVisInfo.setDetails(this->m_panelWidget->getSigDetailsInput().toStdString());
		this->ref_scn->get()->getSigEditor().sigSet_setVisInfo(newSigSetId, std::move(tempVisInfo));
	}
	catch (std::exception& e) {
		//print fail msg in the widget
		this->m_panelWidget->sigFailMessage({ e.what() });
		//early exit as operation failed
		return;
	}

	//overwrite run rec visInfo
	auto* recViSInfo = this->ref_runManager->getVisinfo(this->m_savedIndex);
	recViSInfo->removeSecret("SigSet");
	recViSInfo->addSecret("SigSet", std::to_string(newSigSetId).c_str());

	//display the results
	this->m_panelWidget->sigSaved(newSigSetId);
}

void DRunGAFunction::cancel() {
	//remove the panel
	this->ref_dockWidgetHandler->removeAndDeleteRightDockWidget();
	//re-enable stuff
	this->ref_baseNaviFunction->setIgnoreEvents(false);
	this->ref_tabToolBar->setEnabled(true);

	this->closeFunction();
}

void DRunGAFunction::closeAndOpenRecView() {
	//remove the panel
	this->ref_dockWidgetHandler->removeAndDeleteRightDockWidget();
	//re-enable stuff
	this->ref_baseNaviFunction->setIgnoreEvents(false);
	this->ref_tabToolBar->setEnabled(true);

	this->ref_functionHandler->addFunction(new DPlayBackFunction{ this->m_savedIndex });
	this->closeFunction();
}

// --- --- --- --- --- Private Utils --- --- --- --- ---

void DRunGAFunction::showDefaultSettings() {
	if (this->ref_dParam->m_runGAParams.runTime != -1) {

		//find the sigSetIndex
		int sigSetIndex = 0;
		for (auto& sigSetTextEntry : this->m_sigSetStrings) {
			if (sigSetTextEntry.second == this->ref_dParam->m_runGAParams.sigsetID) {
				//only show if found
				this->m_panelWidget->setSigSetInput(sigSetIndex);
				break;
			}
			sigSetIndex++;
		}

		//find the dmdSetIndex
		int dmdSetIndex = 0;
		for (auto& dmdSetTextEntry : this->m_dmdSetStrings) {
			if (dmdSetTextEntry.second == this->ref_dParam->m_runGAParams.dmdsetID) {
				this->m_panelWidget->setDmdSetInput(dmdSetIndex);
				break;
			}
			dmdSetIndex++;
		}

		//all the other stuff
		this->m_panelWidget->setTimeInput(this->ref_dParam->m_runGAParams.runTime);
		this->m_panelWidget->setGAType(this->ref_dParam->m_runGAParams.gaType);
		this->m_panelWidget->setNumOfThreads(this->ref_dParam->m_runGAParams.threadNum);
		this->m_panelWidget->setNumOfGen(this->ref_dParam->m_runGAParams.genNum);
		this->m_panelWidget->setNumOfSubGen(this->ref_dParam->m_runGAParams.subGenNum);
		this->m_panelWidget->setPopSize(this->ref_dParam->m_runGAParams.popSize);
		this->m_panelWidget->setEliteRate(this->ref_dParam->m_runGAParams.eliteRate);
		this->m_panelWidget->setCrossoverRate(this->ref_dParam->m_runGAParams.crossoverRate);
		this->m_panelWidget->setMutationRate(this->ref_dParam->m_runGAParams.mutationRate);
		this->m_panelWidget->setPowerFactor(this->ref_dParam->m_runGAParams.powerFactor);
		this->m_panelWidget->setOptCyc(this->ref_dParam->m_runGAParams.isoptCyc);
		this->m_panelWidget->setOptOffset(this->ref_dParam->m_runGAParams.isoptOffset);
		this->m_panelWidget->setOptGreen(this->ref_dParam->m_runGAParams.isoptGreen);
		this->m_panelWidget->setMatchCyc(this->ref_dParam->m_runGAParams.ismatchCyc);
	}
}

QList<QString> DRunGAFunction::genSigString() {
	QList<QString> out{};
	for (auto& sigId : this->ref_scn->get()->getSignal().getSigSetIds()) {
		QString str{ std::string{std::to_string(sigId) + " " + this->ref_scn->get()->getSignal().getSigSet(sigId).getVisInfo().getName()}.c_str() };
		this->m_sigSetStrings.emplace(str, sigId);
		out.append(str);
	}
	return out;
}

QList<QString> DRunGAFunction::genDmdString() {
	QList<QString> out{};
	for (auto& dmdId : this->ref_scn->get()->getDemand().getDemandSetIds()) {
		QString str{ std::string{std::to_string(dmdId) + " " + this->ref_scn->get()->getDemand().getDemandSet(dmdId).getVisInfo().getName()}.c_str() };
		this->m_dmdSetStrings.emplace(str, dmdId);
		out.append(str);
	}
	return out;
}
