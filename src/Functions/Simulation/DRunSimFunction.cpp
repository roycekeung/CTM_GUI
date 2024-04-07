#include "DRunSimFunction.h"

#include "PanelWidgets/Simulation/DSimSettingsPanel.h"
#include "DDockWidgetHandler.h"
#include "DTabToolBar.h"
#include "DRunManager.h"
#include "DFunctionHandler.h"
#include "GraphicsItems/DGraphicsScene.h"
#include "Functions/DBaseNaviFunction.h"
#include "Functions/Simulation/DPlayBackFunction.h"

#include "I_SigRecord.h"

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DRunSimFunction::DRunSimFunction() : QObject(nullptr), I_Function() {
}

DRunSimFunction::~DRunSimFunction() {
}

void DRunSimFunction::initFunctionHandler() {
	//setup the panel widget
	this->m_panelWidget = new DSimSettingsPanel(genSigString(), genDmdString());
	this->ref_dockWidgetHandler->setRightDockWidget(this->m_panelWidget, "Run Sim");
	this->m_panelWidget->setupButtons(this);
	this->showDefaultSettings();

	//set up the graphics scene
	this->ref_graphicsScene->clearSelection();

	//disable other stuff
	this->ref_baseNaviFunction->setIgnoreEvents(true);
	this->ref_tabToolBar->setEnabled(false);
}

// --- --- --- --- --- Checking and creation Functions --- --- --- --- ---

void DRunSimFunction::checkAndRunSim() {
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

	try {
		sigSet = this->m_sigSetStrings.at(this->m_panelWidget->getSigSetInput());
		dmdSet = this->m_dmdSetStrings.at(this->m_panelWidget->getDmdSetInput());

		this->ref_scn->get()->getSimBuilder()
			.useSigSet(sigSet)
			.useDmdSet(dmdSet)
			.setSimEndTime((int64_t)this->m_panelWidget->getTimeInput() * 1000)
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
	tempVisInfo.addSecret("SigSet", std::to_string(sigSet));
	tempVisInfo.addSecret("DmdSet", std::to_string(dmdSet));
	tempVisInfo.addSecret("Time", std::to_string(this->m_panelWidget->getTimeInput()));

	//sigRec
	std::unordered_map<int, std::unordered_map<int, std::pair<std::vector<int64_t>, std::vector<int64_t>>>> tSigRec;
	for (auto sigId : this->ref_scn->get()->getSignal().getSigSet(sigSet).getSigIds()) {
		const DISCO2_API::I_SigRecord* sigRec = dynamic_cast<const DISCO2_API::I_SigRecord*>(this->ref_scn->get()->getSignal().getSigSet(sigSet).getSigCtrl(sigId));
		if (sigRec)
			tSigRec.emplace(sigId, sigRec->getAllOnOffTimes());
	}

	this->m_savedIndex = this->ref_runManager->addRec(
		this->ref_scn->get()->getRecHandler()->deepClone(), 
		std::move(tSigRec),
		std::move(tempVisInfo));

	//display the results
	auto* rec = this->ref_runManager->getRec(this->m_savedIndex);
	this->m_panelWidget->showResults(true,
		rec->getTotalDelay() / 1000,
		rec->getRealisedDemand(),
		rec->getAvgDelay() / 1000);
}

void DRunSimFunction::cancel() {
	//remove the panel
	this->ref_dockWidgetHandler->removeAndDeleteRightDockWidget();
	//re-enable stuff
	this->ref_baseNaviFunction->setIgnoreEvents(false);
	this->ref_tabToolBar->setEnabled(true);

	this->closeFunction();
}

void DRunSimFunction::closeAndOpenRecView() {
	//remove the panel
	this->ref_dockWidgetHandler->removeAndDeleteRightDockWidget();
	//re-enable stuff
	this->ref_baseNaviFunction->setIgnoreEvents(false);
	this->ref_tabToolBar->setEnabled(true);

	this->ref_functionHandler->addFunction(new DPlayBackFunction{ this->m_savedIndex });
	this->closeFunction();
}

// --- --- --- --- --- Private Utils --- --- --- --- ---

void DRunSimFunction::showDefaultSettings() {
	if (this->ref_dParam->m_runSimParams.runTime != -1) {

		//find the sigSetIndex
		int sigSetIndex = 0;
		for (auto& sigSetTextEntry : this->m_sigSetStrings) {
			if (sigSetTextEntry.second == this->ref_dParam->m_runSimParams.sigsetID) {
				//only show if found
				this->m_panelWidget->setSigSetInput(sigSetIndex);
				break;
			}
			sigSetIndex++;
		}
		
		//find the dmdSetIndex
		int dmdSetIndex = 0;
		for (auto& dmdSetTextEntry : this->m_dmdSetStrings) {
			if (dmdSetTextEntry.second == this->ref_dParam->m_runSimParams.dmdsetID) {
				this->m_panelWidget->setDmdSetInput(dmdSetIndex);
				break;
			}
			dmdSetIndex++;
		}

		this->m_panelWidget->setTimeInput(this->ref_dParam->m_runSimParams.runTime);
	}
}

QList<QString> DRunSimFunction::genSigString() {
	QList<QString> out{};
	for (auto& sigId : this->ref_scn->get()->getSignal().getSigSetIds()) {
		QString str{ std::string{std::to_string(sigId) + " " + this->ref_scn->get()->getSignal().getSigSet(sigId).getVisInfo().getName()}.c_str() };
		this->m_sigSetStrings.emplace(str, sigId);
		out.append(str);
	}
	return out;
}

QList<QString> DRunSimFunction::genDmdString() {
	QList<QString> out{};
	for (auto& dmdId : this->ref_scn->get()->getDemand().getDemandSetIds()) {
		QString str{ std::string{std::to_string(dmdId) + " " + this->ref_scn->get()->getDemand().getDemandSet(dmdId).getVisInfo().getName()}.c_str() };
		this->m_dmdSetStrings.emplace(str, dmdId);
		out.append(str);
	}
	return out;
}
