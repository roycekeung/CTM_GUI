#pragma once

#include <vector>
//Qt stuff
#include <QWidget>
#include "ui_DSplitCellPanel.h"
//DISCO GUI stuff
class DOkCancelButtons;
class DAddRemoveButtons;
class DSplitCellFunction;

class DSplitCellPanel : public QWidget {
	Q_OBJECT

private:

	// --- --- --- --- --- UI Items --- --- --- --- ---

	Ui::DSplitCellPanel ui;

	DOkCancelButtons* ok_Buttons = nullptr;
	DAddRemoveButtons* add_Buttons = nullptr;

	// --- --- --- --- --- Ref Values --- --- --- --- ---

	int ref_numOfLane = 0;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DSplitCellPanel(QWidget *parent = Q_NULLPTR);

	~DSplitCellPanel();

	// --- --- --- --- --- Settings for Functions --- --- --- --- ---

	void setShowing(bool isShow);

	void setParams(int arcId, int rowNum, int numOfLanes, const std::vector<int>& splits);
	
	void setupButtons(DSplitCellFunction* function);

	void setSplits(const std::vector<int>& splits);

	std::vector<int> getSplits();

	void failMessage(QString&& msg);

signals:

	// --- --- --- --- --- Signals for Real Time Graphics Change --- --- --- --- ---

	void splitsInputChanged(std::vector<int> splits);

	void cancelSplit();

	void cancel();

public slots:

	// --- --- --- --- --- Slots for Real Time Change --- --- --- --- ---

	void splitsChanged();

	//adds row depends on selected row
	void addRow();

	//deletes row depends on selected row
	void removeRows();

	//reply cancel clicked depends on if split is in progress or not
	void clickedCancel();

private:

	// --- --- --- --- --- Private Utils --- --- --- --- ---

	void addRowToTable(int id, int laneCount);

	void resetMaximum();

	int currentInputTotal();
	
};
