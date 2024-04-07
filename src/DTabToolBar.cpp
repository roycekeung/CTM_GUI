#include "DTabToolBar.h"

#include "DFunctionHandler.h"

#include "Functions/File/DLoadScenFunc.h"
#include "Functions/File/DNewScenFunc.h"
#include "Functions/File/DSaveScenFunc.h"
#include "Functions/File/DSigDemandFileFunction.h"

#include "Functions/Home/DDefaultParamSettingFunction.h"
#include "Functions/Home/DPanToggleFunction.h"
#include "Functions/Home/DZoomToExtentFunction.h"
#include "Functions/Home/DShowHideFunction.h"
#include "Functions/Home/DShowHideBaseMapFunction.h"
#include "Functions/Home/DListElementFunction.h"

#include "Functions/Network/DNetLockFunction.h"
#include "Functions/Network/DDeleteModeFunction.h"
#include "Functions/Network/DArcCreateFunction.h"
#include "Functions/Network/DDemandSinkCellCreateFunc.h"
#include "Functions/Network/DAddNewConnectorFunction.h"
#include "Functions/Network/DRemoveConnectorFunction.h"
#include "Functions/Network/DSplitCellFunction.h"

#include "Functions/Junction/DJctCreateFunction.h"
#include "Functions/Junction/DJctLinkCreateFunction.h"
#include "Functions/Junction/DDeleteJctLinkFunction.h"
#include "Functions/Junction/DSigGroupsFunction.h"
#include "Functions/Junction/DSignalTimingPlanCreateFunc.h"
#include "Functions/Junction/DConflictMatrixFunction.h"

#include "Functions/Simulation/DRunSimFunction.h"
#include "Functions/Simulation/DRunGAFunction.h"
#include "Functions/Simulation/DManageRunFunction.h"

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DTabToolBar::DTabToolBar(QWidget *parent, DFunctionHandler* functionHandler) : QWidget(parent), ref_functionHandler(functionHandler) {
	ui.setupUi(this);
	this->connectAllButtons();

	//hide unused tabs
	this->setJctTab(false);
	this->setPlaybackTab(false);

	//set tab to home tab
	this->ui.DToolbarTabWidget->setCurrentIndex(1);
	
	//hide un-implemented stuff in deploy
#ifdef DEPLOY_GUI
	//home tab
	this->ui.ListElementsButton->setVisible(false);
	this->ui.DetailsButton->setVisible(false);
	this->ui.SelectButton->setVisible(false);
	this->ui.ErrorListButton->setVisible(false);
	this->ui.DownloadOSMapButton->setVisible(false);
	this->ui.AdjustMapButton->setVisible(false);

	//Network tab
	this->ui.SplitArcButton->setVisible(false);

	//Jct tab
	this->ui.ListJctLink_Button->setVisible(false);

	//sim tab
	this->ui.genDelayPlot_pushButton->setVisible(false);
#endif // DEPLOY_GUI

#ifdef LOCKED_GUI_DEPLOY
	//Jct tab
	this->ui.conflictMatrix_pushButton->setVisible(false);
#endif // LOCKED_GUI_DEPLOY

}

DTabToolBar::~DTabToolBar() {

}

void DTabToolBar::connectAllButtons() {
	//File Tab
	QObject::connect(this->ui.OpenButton, &QPushButton::clicked, this, &DTabToolBar::ShowLoadScenDialog);
	QObject::connect(this->ui.NewButton, &QPushButton::clicked, this, &DTabToolBar::ShowNewScenDialog);
	QObject::connect(this->ui.SaveButton, &QPushButton::clicked, this, &DTabToolBar::ShowSaveScenDialog);
	QObject::connect(this->ui.SignalPlansButton, &QPushButton::clicked, this, &DTabToolBar::ShowSigFile);
	QObject::connect(this->ui.DemandProfileButton, &QPushButton::clicked, this, &DTabToolBar::ShowDemFile);

	//Home
	QObject::connect(this->ui.SettingsButton, &QPushButton::clicked, this, &DTabToolBar::createDefaultParamSettingFunction);
	QObject::connect(this->ui.PanButton, &QPushButton::toggled, this, &DTabToolBar::createPanFunction);
	QObject::connect(this->ui.ZoomButton, &QPushButton::clicked, this, &DTabToolBar::createZoomToExtentFunction);
	QObject::connect(this->ui.ShowHideButton, &QPushButton::clicked, this, &DTabToolBar::createShowHideFunction);
	QObject::connect(this->ui.OnOffButton, &QPushButton::clicked, this, &DTabToolBar::showHideBaseMapFunction);
	QObject::connect(this->ui.ListElementsButton, &QPushButton::clicked, this, &DTabToolBar::createListElementFunction);

	//Network
	QObject::connect(this->ui.LockUnlockButton, &QPushButton::clicked, this, &DTabToolBar::netLockFunction);
	QObject::connect(this->ui.delete_pushButton, &QPushButton::clicked, this, &DTabToolBar::deleteModeFunction);
	QObject::connect(this->ui.CreateArcButton, &QPushButton::clicked, this, &DTabToolBar::createArcFunction);
	QObject::connect(this->ui.CreateDemandSinkCellButton, &QPushButton::clicked, this, &DTabToolBar::createDemandSinkCellFunction);
	QObject::connect(this->ui.CreateJunctionNodeButton, &QPushButton::clicked, this, &DTabToolBar::createJctFunction);
	QObject::connect(this->ui.NewConnectorButton, &QPushButton::clicked, this, &DTabToolBar::createAddNewConnectorFunction);
	QObject::connect(this->ui.RemoveConnectorButton, &QPushButton::clicked, this, &DTabToolBar::createRemoveConnectorFunction);
	QObject::connect(this->ui.SplitCellButton, &QPushButton::clicked, this, &DTabToolBar::createSplitCellsFunction);

	//Junction
	QObject::connect(this->ui.NewJctLink_Button, &QPushButton::clicked, this, &DTabToolBar::createJctLinkFunction);
	QObject::connect(this->ui.DeleteJctLink_Button, &QPushButton::clicked, this, &DTabToolBar::deleteJctLinkFunction);
	QObject::connect(this->ui.SignalGroups_Button, &QPushButton::clicked, this, &DTabToolBar::signalGroupsFunction);
	QObject::connect(this->ui.SignalTimingPlan_Button, &QPushButton::clicked, this, &DTabToolBar::createsignalTimingFunction);
	QObject::connect(this->ui.conflictMatrix_pushButton, &QPushButton::clicked, this, &DTabToolBar::conflictMatrixFunction);

	//Simulation
	QObject::connect(this->ui.runSim_pushButton, &QPushButton::clicked, this, &DTabToolBar::runSimFunction);
	QObject::connect(this->ui.runOpt_pushButton, &QPushButton::clicked, this, &DTabToolBar::runGAFunction);
	QObject::connect(this->ui.manageRuns_pushButton, &QPushButton::clicked, this, &DTabToolBar::manageRunFunction);

}

// --- --- --- --- --- Get Button For Connection --- --- --- --- ---

QPushButton* DTabToolBar::getExitJctButton() {
	return this->ui.ExitJctView_Button;
}

QPushButton* DTabToolBar::getExitPlayBackButton() {
	return this->ui.exitPlayback_pushButton;
}

QPushButton* DTabToolBar::getPlaybackOptionsButton() {
	return this->ui.playbackOptions_pushButton;
}

QPushButton* DTabToolBar::getPlaybackTimeBarButton() {
	return this->ui.playbackTimeBar_pushButton;
}

QPushButton* DTabToolBar::getPlaybackGenDelayPlotButton() {
	return this->ui.genDelayPlot_pushButton;
}

QPushButton* DTabToolBar::getPlaybackGenQueuePlotButton() {
	return this->ui.genQueuePlot_pushButton;
}

// --- --- --- --- --- File Tab --- --- --- --- ---

void DTabToolBar::ShowLoadScenDialog() {
	this->ref_functionHandler->addFunction(new DLoadScenFunc{});
}

void DTabToolBar::ShowNewScenDialog() {
	this->ref_functionHandler->addFunction(new DNewScenFunc{});
}

void DTabToolBar::ShowSaveScenDialog() {
	this->ref_functionHandler->addFunction(new DSaveScenFunc{});
}

void DTabToolBar::ShowSigFile() {
	this->ref_functionHandler->addFunction(new DSigDemandFileFunction{ DSigDemandFileFunction::Type_File::Signal });
}

void DTabToolBar::ShowDemFile() {
	this->ref_functionHandler->addFunction(new DSigDemandFileFunction{ DSigDemandFileFunction::Type_File::Demand });
}

// --- --- --- --- --- Home Tab --- --- --- --- ---
void DTabToolBar::createDefaultParamSettingFunction() {
	this->ref_functionHandler->addFunction(new DDefaultParamSettingFunction());
}
void DTabToolBar::createPanFunction(bool checked) {
	if (checked) {
		this->ref_functionHandler->addFunction(new DPanToggleFunction(this->ui.PanButton));
	}
}
void DTabToolBar::createZoomToExtentFunction() {
	this->ref_functionHandler->addFunction(new DZoomToExtentFunction());
}

void DTabToolBar::createShowHideFunction() {
	this->ref_functionHandler->addFunction(new DShowHideFunction());
}

void DTabToolBar::showHideBaseMapFunction() {
	this->ref_functionHandler->addFunction(new DShowHideBaseMapFunction(this->ui.OnOffButton));
}

void DTabToolBar::createListElementFunction() {
	this->ref_functionHandler->addFunction(new DListElementFunction());
}

// --- --- --- --- --- Network Tab --- --- --- --- ---

void DTabToolBar::netLockFunction() {
	this->ref_functionHandler->addFunction(new DNetLockFunction{ this->ui.LockUnlockButton });
}

void DTabToolBar::deleteModeFunction() {
	this->ref_functionHandler->addFunction(new DDeleteModeFunction{ this->ui.delete_pushButton });
}

void DTabToolBar::createArcFunction() {
	this->ref_functionHandler->addFunction(new DArcCreateFunction{});
}

void DTabToolBar::createDemandSinkCellFunction() {
	this->ref_functionHandler->addFunction(new DDemandSinkCellCreateFunc{});
}

void DTabToolBar::createAddNewConnectorFunction() {
	this->ref_functionHandler->addFunction(new DAddNewConnectorFunction{});
}

void DTabToolBar::createRemoveConnectorFunction() {
	this->ref_functionHandler->addFunction(new DRemoveConnectorFunction{});
}

void DTabToolBar::createJctFunction() {
	this->ref_functionHandler->addFunction(new DJctCreateFunction{});
}

void DTabToolBar::createSplitCellsFunction() {
	this->ref_functionHandler->addFunction(new DSplitCellFunction{});
}

// --- --- --- --- --- Junction Tab --- --- --- --- ---

void DTabToolBar::setJctTab(bool isOn, bool hasSig, bool hasBox) {
	//jct Tab
	this->ui.DToolbarTabWidget->setTabVisible(tabIndex::jctTab, isOn);
	this->ui.DToolbarTabWidget->setCurrentIndex(isOn ? tabIndex::jctTab : tabIndex::netTab);
	//all other tabs
	for (int i = 0; i < this->ui.DToolbarTabWidget->count(); ++i) {
		if (i != tabIndex::jctTab)
			this->ui.DToolbarTabWidget->setTabEnabled(i, !isOn);
	}
	//enable/disable button if needed
	this->ui.SignalGroups_Button->setEnabled(hasSig);
	this->ui.SignalTimingPlan_Button->setEnabled(hasSig);
}

void DTabToolBar::createJctLinkFunction() {
	this->ref_functionHandler->addFunction(new DJctLinkCreateFunction{});
}

void DTabToolBar::deleteJctLinkFunction() {
	this->ref_functionHandler->addFunction(new DDeleteJctLinkFunction{this->ui.DeleteJctLink_Button});
}

void DTabToolBar::signalGroupsFunction() {
	this->ref_functionHandler->addFunction(new DSigGroupsFunction{});
}

void DTabToolBar::createsignalTimingFunction() {
	this->ref_functionHandler->addFunction(new DSignalTimingPlanCreateFunc{});
}

void DTabToolBar::conflictMatrixFunction() {
	this->ref_functionHandler->addFunction(new DConflictMatrixFunction{});
}

// --- --- --- --- --- Simulation Tab --- --- --- --- ---

void DTabToolBar::runSimFunction() {
	this->ref_functionHandler->addFunction(new DRunSimFunction{});
}

void DTabToolBar::runGAFunction() {
	this->ref_functionHandler->addFunction(new DRunGAFunction{});
}

void DTabToolBar::manageRunFunction() {
	this->ref_functionHandler->addFunction(new DManageRunFunction{});
}

// --- --- --- --- --- Playback Tab --- --- --- --- ---

void DTabToolBar::setPlaybackTab(bool isOn) {
	//Playback Tab
	this->ui.DToolbarTabWidget->setTabVisible(tabIndex::playbackTab, isOn);
	this->ui.DToolbarTabWidget->setCurrentIndex(isOn ? tabIndex::playbackTab : tabIndex::simTab);
	//all other tabs
	for (int i = 0; i < this->ui.DToolbarTabWidget->count(); ++i) {
		if (i != tabIndex::playbackTab)
			this->ui.DToolbarTabWidget->setTabEnabled(i, !isOn);
	}
}
