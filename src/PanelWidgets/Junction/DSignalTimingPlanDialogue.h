#pragma once

#include <QDialog>
#include "ui_DSignalTimingPlanDialogue.h"


// pre def
class DOkCancelButtons;
class DSignalTimingPlanCreateFunc;
class MouseWheelWidgetAdjustmentGuard;

class DSignalTimingPlanDialogue : public QDialog {
	Q_OBJECT

private:
	// --- --- --- --- --- Panel UI --- --- --- --- ---
	Ui::DSignalTimingPlanDialogue ui;
	DOkCancelButtons* ok_Buttons = nullptr;

	MouseWheelWidgetAdjustmentGuard* noMoreScrolling = nullptr;;

	// --- --- --- --- --- Private Utils --- --- --- --- ---
	// insert external button, text
	void insertOkButtons();
	void updateMaxLimitToGreenOnOffTime();

public:
	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---
	DSignalTimingPlanDialogue();
	~DSignalTimingPlanDialogue();


	// --- --- --- --- --- Settings for Functions --- --- --- --- ---
	void setupCreateButtons(DSignalTimingPlanCreateFunc* function);
	void setref_confirmButtonEnabled(bool IsEnable);
	void enableConfirmButton();
	void connectSigSetComboBox(DSignalTimingPlanCreateFunc* function);
	void connectDataChanged(DSignalTimingPlanCreateFunc* function);

	//why are all the connection separated into so many functions -JLo

	void setCycleTime(int CycleTime);
	void setOffset(int OffsetTime);
	void setSigSetComboBoxIndex(int index);

	//  --- --- --- --- --- provide direct interaction with the TimingIput_tableWidget  --- --- --- --- ---
	void ClearAllContentInTable();
	void addRowToTable(DSignalTimingPlanCreateFunc* function, int SigGpID, int GreenOnTime, int GreenOffTime, int Green2OnTime, int Green2OffTime);
	void OverwriteRowToTableRegardsToSigGpID( int SigGpID, int GreenOnTime, int GreenOffTime, int Green2OnTime, int Green2OffTime);

	// --- --- --- --- --- show error tool --- --- --- --- --- 
	void failMessage(QString&& msg);

	//// --- --- --- --- --- getter --- --- --- --- ---
	int getCycleTime();
	int getOffset();
	int getSigSetComboBoxIndex();

	Ui::DSignalTimingPlanDialogue getUI();

};
