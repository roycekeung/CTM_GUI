#pragma once

#include <QWidget>

#include "ui_DTabToolBar.h"

class DFunctionHandler;
class QPushButton;

class DAddNewConnectorFunction;
class DRemoveConnectorFunction;

class DTabToolBar : public QWidget {
	Q_OBJECT

private:

	enum tabIndex { fileTab, homeTab, netTab, simTab, jctTab, playbackTab };

	DFunctionHandler* ref_functionHandler;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DTabToolBar(QWidget *parent, DFunctionHandler* functionHandler);

	~DTabToolBar();

private:
	Ui::DTabToolBar ui;

	void connectAllButtons();

public:

	// --- --- --- --- --- Get Button For Connection --- --- --- --- ---

	QPushButton* getExitJctButton();
	QPushButton* getExitPlayBackButton();
	QPushButton* getPlaybackOptionsButton();
	QPushButton* getPlaybackTimeBarButton();
	QPushButton* getPlaybackGenDelayPlotButton();
	QPushButton* getPlaybackGenQueuePlotButton();

public slots:

	// --- --- --- --- --- File Tab --- --- --- --- ---

	void ShowLoadScenDialog();
	void ShowNewScenDialog();
	void ShowSaveScenDialog();

	void ShowSigFile();
	void ShowDemFile();

	// --- --- --- --- --- Home Tab --- --- --- --- ---
	void createDefaultParamSettingFunction();
	void createPanFunction(bool checked);
	void createZoomToExtentFunction();
	void createShowHideFunction();
	void showHideBaseMapFunction();
	void createListElementFunction();

	// --- --- --- --- --- Network Tab --- --- --- --- ---

	void netLockFunction();
	void deleteModeFunction();
	void createArcFunction();
	void createDemandSinkCellFunction();
	void createAddNewConnectorFunction();
	void createRemoveConnectorFunction();
	void createJctFunction();
	void createSplitCellsFunction();

	// --- --- --- --- --- Junction Tab --- --- --- --- ---

	void setJctTab(bool isOn, bool hasSig = false, bool hasBox = false);
	void createJctLinkFunction();
	void deleteJctLinkFunction();
	void signalGroupsFunction();
	void createsignalTimingFunction();
	void conflictMatrixFunction();

	// --- --- --- --- --- Simulation Tab --- --- --- --- ---

	void runSimFunction();
	void runGAFunction();
	void manageRunFunction();

	// --- --- --- --- --- Playback Tab --- --- --- --- ---

	void setPlaybackTab(bool isOn);

};
