#pragma once

#include <QDialog>
#include "ui_DDefaultParamSettingDialogue.h"

// pre def
class DOkCancelButtons;
class DDefaultParamSettingFunction;
class MouseWheelWidgetAdjustmentGuard;
#include "../Utils/DDefaultParamData.h"


class DDefaultParamSettingDialogue : public QDialog{
	Q_OBJECT
private:
	// --- --- --- --- --- Panel UI --- --- --- --- ---
	Ui::DDefaultParamSettingDialogue ui;
	DOkCancelButtons* ok_Buttons = nullptr;

	// --- --- --- --- --- Private Utils --- --- --- --- ---
	// insert external button, text
	void insertOkButtons();


public:
	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---
	DDefaultParamSettingDialogue(const QList<QString>& sigSetEntries, const QList<QString>& dmdSetEntries,
		DefaultParamDataSet* dparam);
	~DDefaultParamSettingDialogue();


	// --- --- --- --- --- Settings for Functions --- --- --- --- ---
	void setupOKCancelButtons(DDefaultParamSettingFunction* function);
	void setEnableConfirmButton(bool enable);

	// --- --- --- --- --- show error tool --- --- --- --- --- 
	void failMessage(QString&& msg);

	// --- --- --- --- --- Set values --- --- --- --- ---
	// Default Setting for Flow Parameters
	void setNumberOfLane(int numOfLane);
	void setFreeFlowSpeed(double v);
	void setBackwardShockwaveSpeed(double w);
	void setJamDensity(double k);
	void setSaturationFlow(double q);
	void setisShowLastInput(bool enable);

	// Default Setting for Sim
	void setSimSigSetID(int setID);
	void setSimDmdSetID(int setID);
	void setSimTimeInput(int time);

	// Default Setting for GA
	void setGASigSetID(int setID);
	void setGADmdSetID(int setID);
	void setGATimeInput(int time);

	void setGAType(int Index);
	void setNumOfThreads(int numOfThread);
	void setGANumOfGen(int numOfgen);
	void setGANumOfSubGen(int numOfsubgen);
	void setGAPopSize(int numOfpop);
	void setGAEliteRate(double rate);
	void setGACrossoverRate(double rate);
	void setGAMutationRate(double rate);
	void setGAPowerFactor(double rate);
	void setGAOptCyc(bool enable);
	void setGAOptOffset(bool enable);
	void setGAOptGreen(bool enable);
	void setGAMatchCyc(bool enable);


	// --- --- --- --- --- update all values --- --- --- --- ---
	// pass in the struct DefaultParamDataSet pointer to replicate down into each elements
	void updateAllValues(DefaultParamDataSet* dparam);


	// --- --- --- --- --- getter --- --- --- --- ---
	// get Default Flow Parameters
	int getNumberOfLane();
	double getFreeFlowSpeed() ;
	double getBackwardShockwaveSpeed();
	double getJamDensity() ;
	double getSaturationFlow() ;
	bool getisShowLastInput() ;

	// get Default Sim Parameters
	QString getSimSigSetInput();
	int getSimSigSetID();
	QString getSimDmdSetInput();
	int getSimDmdSetID();
	int getSimTimeInput();

	// get Default GA Parameters
	QString getGASigSetInput();
	int getGASigSetID();
	QString getGADmdSetInput();
	int getGADmdSetID();
	int getGATimeInput();

	int getGAType();
	int getNumOfThreads();
	int getGANumOfGen();
	int getGANumOfSubGen();
	int getGAPopSize();
	double getGAEliteRate();
	double getGACrossoverRate();
	double getGAMutationRate();
	double getGAPowerFactor();
	bool getGAOptCyc();
	bool getGAOptOffset();
	bool getGAOptGreen();
	bool getGAMatchCyc();


};
