#include "DDefaultParamSettingFunction.h"

// std lib
#include <unordered_map>

// DISCO_GUI lib
#include "../PanelWidgets/Home/DDefaultParamSettingDialogue.h"
#include "DTabToolBar.h"
#include "../DBaseNaviFunction.h"
#include "../Utils/DDefaultParamData.h"

// Qt lib
#include <QString>
#include <QList>


// --- --- --- --- --- Private Utils --- --- --- --- ---
QList<QString> DDefaultParamSettingFunction::genSigString() {
	QList<QString> out{};
	for (auto& sigId : this->ref_scn->get()->getSignal().getSigSetIds()) {
		QString str{ std::string{std::to_string(sigId) + " " + this->ref_scn->get()->getSignal().getSigSet(sigId).getVisInfo().getName()}.c_str() };
		this->m_sigSetStrings.emplace(str, sigId);
		out.append(str);
	}
	return out;
}

QList<QString> DDefaultParamSettingFunction::genDmdString() {
	QList<QString> out{};
	for (auto& dmdId : this->ref_scn->get()->getDemand().getDemandSetIds()) {
		QString str{ std::string{std::to_string(dmdId) + " " + this->ref_scn->get()->getDemand().getDemandSet(dmdId).getVisInfo().getName()}.c_str() };
		this->m_dmdSetStrings.emplace(str, dmdId);
		out.append(str);
	}
	return out;
}

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---
DDefaultParamSettingFunction::DDefaultParamSettingFunction() :QObject(nullptr) {



}

DDefaultParamSettingFunction::~DDefaultParamSettingFunction(){


}



// --- --- --- --- --- override initFunctionHandler from I_Function --- --- --- --- ---
void DDefaultParamSettingFunction::initFunctionHandler() {
	// disable the tabtoolbar first, no other button can be clicked
	this->ref_tabToolBar->setEnabled(false);

	this->m_DialogueWigdet = new DDefaultParamSettingDialogue(genSigString(), genDmdString(), this->ref_dParam);
	this->m_DialogueWigdet->setupOKCancelButtons(this);

	// turn off the built-in help question mark button
	this->m_DialogueWigdet->setWindowFlags(this->m_DialogueWigdet->windowFlags() & ~Qt::WindowContextHelpButtonHint);

	// run dialog
	this->m_DialogueWigdet->exec();
}


// --- --- --- --- --- Checking and creation Functions --- --- --- --- ---
void DDefaultParamSettingFunction::checkAndSubmitChanges() {
	try {
		// record the changes on Default Flow Parameters
		this->ref_dParam->m_FlowParams.NumberOfLane = this->m_DialogueWigdet->getNumberOfLane();
		this->ref_dParam->m_FlowParams.FreeFlowSpeed = this->m_DialogueWigdet->getFreeFlowSpeed();
		this->ref_dParam->m_FlowParams.BackwardShockwaveSpeed = this->m_DialogueWigdet->getBackwardShockwaveSpeed();
		this->ref_dParam->m_FlowParams.JamDensity = this->m_DialogueWigdet->getJamDensity();
		this->ref_dParam->m_FlowParams.SaturationFlow = this->m_DialogueWigdet->getSaturationFlow();
		this->ref_dParam->isShowLastInput = this->m_DialogueWigdet->getisShowLastInput();

		// record the changes on Default Sim Parameters
		this->ref_dParam->m_runSimParams.sigsetID = this->m_DialogueWigdet->getSimSigSetID();
		this->ref_dParam->m_runSimParams.dmdsetID = this->m_DialogueWigdet->getSimDmdSetID();
		this->ref_dParam->m_runSimParams.runTime = this->m_DialogueWigdet->getSimTimeInput();

		// record the changes on Default GA Parameters
		this->ref_dParam->m_runGAParams.sigsetID = this->m_DialogueWigdet->getGASigSetID();
		this->ref_dParam->m_runGAParams.dmdsetID = this->m_DialogueWigdet->getGADmdSetID();
		this->ref_dParam->m_runGAParams.runTime = this->m_DialogueWigdet->getGATimeInput();

		this->ref_dParam->m_runGAParams.gaType = this->m_DialogueWigdet->getGAType();
		this->ref_dParam->m_runGAParams.threadNum = this->m_DialogueWigdet->getNumOfThreads();
		this->ref_dParam->m_runGAParams.genNum = this->m_DialogueWigdet->getGANumOfGen();
		this->ref_dParam->m_runGAParams.subGenNum = this->m_DialogueWigdet->getGANumOfSubGen();
		this->ref_dParam->m_runGAParams.popSize = this->m_DialogueWigdet->getGAPopSize();
		this->ref_dParam->m_runGAParams.eliteRate = this->m_DialogueWigdet->getGAEliteRate();
		this->ref_dParam->m_runGAParams.crossoverRate = this->m_DialogueWigdet->getGACrossoverRate();
		this->ref_dParam->m_runGAParams.mutationRate = this->m_DialogueWigdet->getGAMutationRate();
		this->ref_dParam->m_runGAParams.powerFactor = this->m_DialogueWigdet->getGAPowerFactor();
		this->ref_dParam->m_runGAParams.isoptCyc = this->m_DialogueWigdet->getGAOptCyc();
		this->ref_dParam->m_runGAParams.isoptOffset = this->m_DialogueWigdet->getGAOptOffset();
		this->ref_dParam->m_runGAParams.isoptGreen = this->m_DialogueWigdet->getGAOptGreen();
		this->ref_dParam->m_runGAParams.ismatchCyc = this->m_DialogueWigdet->getGAMatchCyc();

	}
	catch (std::exception& e) {
		//print fail msg in the widget
		this->m_DialogueWigdet->failMessage({ e.what() });
	}

}



// --- --- --- --- --- Slots for Panel --- --- --- --- ---
// close func and the dialog
void DDefaultParamSettingFunction::dialogFinished() {
	// the tabtoolbar return normal 
	this->ref_tabToolBar->setEnabled(true);

	this->m_DialogueWigdet->close();
	// end this function
	this->closeFunction();
}

// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---
bool DDefaultParamSettingFunction::keyPressEvent(QKeyEvent* keyEvent) {
	if (keyEvent->key() == Qt::Key::Key_Escape) {
		// close func and the dialog
		dialogFinished();

		return true;
	}
	// continue to handle other events
	return false;
}


