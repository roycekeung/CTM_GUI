#pragma once

#include <QWidget>
#include "ui_DShowHidePanel.h"

// pre def
class QPushButton;
class QCheckBox;

class DShowHideFunction;
class DBaseNaviFunction;

class DShowHidePanel : public QWidget{
	Q_OBJECT
private:
	Ui::DShowHidePanel ui;

	QCheckBox * m_HideDmdSinkCells_CheckBox;
	QCheckBox * m_HideCells_CheckBox;
	QCheckBox * m_HideArcs_CheckBox;
	QCheckBox * m_HideJcts_CheckBox;
				  
	QPushButton* m_HideAll_Button;
	QPushButton* m_UnhideAll_Button;
	QPushButton* m_Done_Button;

	// --- --- --- --- --- inner functions --- --- --- --- ---
	// used to trannslate to viable check state bool that return from the signal of stateChanged
	bool CheckState(int checkState);

public:
	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---
	DShowHidePanel();
	~DShowHidePanel();

	// --- --- --- --- --- Settings for Functions --- --- --- --- ---
	void ConnectButtons(DShowHideFunction* function);
	void ConnectCheckBoxes(DBaseNaviFunction* Scene);
	// set the state of the checkbox
	void SetAllCheckBox(bool checked);


	// --- --- --- --- --- getters --- --- --- --- ---
	QCheckBox* getHideDmdSinkCellsCheckBox();
	QCheckBox* getHideCellsCheckBox();
	QCheckBox* getHideArcsCheckBox();
	QCheckBox* getHideJctsCheckBox();

	QPushButton* getHideAllButton();
	QPushButton* getUnhideAllButton();
	QPushButton* getCancelButton();

	void failMessage(QString&& msg);
};
