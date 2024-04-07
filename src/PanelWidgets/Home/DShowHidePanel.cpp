#include "DShowHidePanel.h"

//DISCO GUI stuff
#include "../Functions/Home/DShowHideFunction.h"
#include "../GraphicsItems/DGraphicsScene.h"
#include "../DBaseNaviFunction.h"

//Qt lib
#include <QString>
#include <QtWidgets/QLabel>
#include <QPushButton>
#include <QCheckBox>

// std lib
#include <string>

// --- --- --- --- --- inner functions --- --- --- --- ---
// used to trannslate to viable check state bool that return from the signal of stateChanged
bool DShowHidePanel::CheckState(int checkState) {
	return (checkState == Qt::CheckState::Checked);
}

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---
DShowHidePanel::DShowHidePanel():QWidget(nullptr){
	ui.setupUi(this);

	// seperate out the buttons and checkbox ptr
	m_HideDmdSinkCells_CheckBox = ui.HideDmdSnkCells_checkBox;
	m_HideCells_CheckBox = ui.HideCells_checkBox;
	m_HideArcs_CheckBox = ui.HideArcs_checkBox;
	m_HideJcts_CheckBox = ui.HideJunctions_checkBox;

	m_HideAll_Button = ui.HideAll_pushButton;
	m_UnhideAll_Button = ui.UnhideAll_pushButton;
	m_Done_Button = ui.Done_Button;
}

DShowHidePanel::~DShowHidePanel(){

}

// --- --- --- --- --- Settings for Functions --- --- --- --- ---
void DShowHidePanel::ConnectButtons(DShowHideFunction* function) {
	//connect buttons to function methods
	QObject::connect(this->m_HideAll_Button, &QPushButton::clicked, this, [=] (){SetAllCheckBox(true);});
	QObject::connect(this->m_UnhideAll_Button, &QPushButton::clicked, this, [=] (){SetAllCheckBox(false); });
	QObject::connect(this->m_Done_Button, &QPushButton::clicked, function, &DShowHideFunction::cancel);

}

void DShowHidePanel::ConnectCheckBoxes(DBaseNaviFunction* BaseNavi) {
	//connect buttons to function methods
	QObject::connect(this->m_HideDmdSinkCells_CheckBox, QOverload<int>::of(&QCheckBox::stateChanged), BaseNavi,
		[=] (int x){BaseNavi->setShowAllDmdSnkCells(!CheckState(x)); });
	QObject::connect(this->m_HideCells_CheckBox, QOverload<int>::of(&QCheckBox::stateChanged), BaseNavi,
		[=] (int x){BaseNavi->setShowAllCells(!CheckState(x)); });
	QObject::connect(this->m_HideArcs_CheckBox, QOverload<int>::of(&QCheckBox::stateChanged), BaseNavi,
		[=] (int x){BaseNavi->setShowAllArcs(!CheckState(x)); });
	QObject::connect(this->m_HideJcts_CheckBox, QOverload<int>::of(&QCheckBox::stateChanged), BaseNavi,
		[=] (int x){BaseNavi->setShowAllJcts(!CheckState(x)); });
}

// set the state of the checkbox
void DShowHidePanel::SetAllCheckBox(bool checked) {
	this->m_HideDmdSinkCells_CheckBox->setChecked(checked);
	this->m_HideCells_CheckBox->setChecked(checked);
	this->m_HideArcs_CheckBox->setChecked(checked);
	this->m_HideJcts_CheckBox->setChecked(checked);
}


// --- --- --- --- --- getters --- --- --- --- ---
QCheckBox* DShowHidePanel::getHideDmdSinkCellsCheckBox() {
	return this->m_HideDmdSinkCells_CheckBox;
}

QCheckBox* DShowHidePanel::getHideCellsCheckBox() {
	return this->m_HideCells_CheckBox;
}

QCheckBox* DShowHidePanel::getHideArcsCheckBox() {
	return this->m_HideArcs_CheckBox;
}

QCheckBox* DShowHidePanel::getHideJctsCheckBox() {
	return this->m_HideJcts_CheckBox;
}


QPushButton* DShowHidePanel::getHideAllButton() {
	return this->m_HideAll_Button;
}

QPushButton* DShowHidePanel::getUnhideAllButton() {
	return this->m_UnhideAll_Button;
}

QPushButton* DShowHidePanel::getCancelButton() {
	return this->m_Done_Button;
}


// --- --- --- --- --- show error tool --- --- --- --- --- 
void DShowHidePanel::failMessage(QString&& msg) {
	this->ui.Feedback_Label->setText(msg);
	this->ui.Feedback_Label->update();
}


