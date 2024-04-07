#include "DJctNodePanel.h"

#include <string>

#include <Jct_Node.h>

#include "Functions/Junction/DJunctionModeFunction.h"
#include "Functions/Junction/DJctCreateFunction.h"
#include "Functions/Junction/DJctEditFunction.h"
#include "Utils/MouseWheelWidgetAdjustmentGuard.h"

// --- --- --- --- --- Hint Texts --- --- --- --- ---

std::string hintTextCreateJct{
"Left click on the map to select the location of the junction\r\n"
};

std::string hintTextEditJct{
"Click and drag the Junction to reposition the Junction\r\n"
};

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DJctNodePanel::DJctNodePanel(Type_JctPanel type, const DISCO2_API::Jct_Node* jctNode) : QWidget(nullptr), ref_jctNode(jctNode) {
	ui.setupUi(this);
	// NO MORE SCROLLING
	auto noMoreScrolling = new MouseWheelWidgetAdjustmentGuard{ this };
	this->ui.Signalized_comboBox->installEventFilter(noMoreScrolling);
	this->ui.Box_Junction_comboBox->installEventFilter(noMoreScrolling);

	switch (type) {
	case Type_JctPanel::create:
		ui.groupBox->setTitle("Create Junction");
		this->insertHintLabel(type);
		this->showJctDefaultParam();
		this->insertOkButtons();
		this->setEditable(false);	//lock input initialy
		break;
	case Type_JctPanel::edit:
		ui.groupBox->setTitle("Edit Junction");
		this->insertHintLabel(type);
		this->showJctIdAndParam();
		this->insertOkButtons();
		this->insertJctModeButton();
		this->setEditable(true);
		break;
	case Type_JctPanel::view:
		ui.groupBox->setTitle("Junction");
		this->showJctIdAndParam();
		this->setEditable(false);
		break;
	}
}

DJctNodePanel::~DJctNodePanel() {
}

// --- --- --- --- --- Settings for Functions --- --- --- --- ---

void DJctNodePanel::setEditable(bool editable) {
	//set all to read only mode
	this->ui.Name_LineEdit->setReadOnly(!editable);
	this->ui.Details_plainTextEdit->setReadOnly(!editable);
	this->ui.Signalized_comboBox->setEnabled(editable);
	this->ui.Box_Junction_comboBox->setEnabled(editable);
}

void DJctNodePanel::enableConfirmButton() {
	this->ok_Buttons->ref_confirmButton->setEnabled(true);
}

void DJctNodePanel::setupCreateButtons(DJctCreateFunction* function) {
	//set the text of the buttons
	this->ok_Buttons->ref_confirmButton->setText("Create");
	this->ok_Buttons->ref_cancelButton->setText("Cancel");
	this->ok_Buttons->ref_confirmButton->setEnabled(false);
	
	//connect buttons to function methods
	QObject::connect(this->ok_Buttons->ref_confirmButton, &QPushButton::clicked, function, &DJctCreateFunction::checkAndCreateJct);
	QObject::connect(this->ok_Buttons->ref_cancelButton, &QPushButton::clicked, function, &DJctCreateFunction::cancel);
}

void DJctNodePanel::setupEditButtons(DJctEditFunction* function) {
	//set the text of the buttons
	this->ok_Buttons->ref_confirmButton->setText("Edit");
	this->ok_Buttons->ref_cancelButton->setText("Cancel");
	this->ok_Buttons->ref_confirmButton->setEnabled(false);

	//connect buttons to function methods
	QObject::connect(this->ok_Buttons->ref_confirmButton, &QPushButton::clicked, function, &DJctEditFunction::checkAndEditJct);
	QObject::connect(this->ok_Buttons->ref_cancelButton, &QPushButton::clicked, function, &DJctEditFunction::cancel);

	//connect internal value changes to the confirm button
	QObject::connect(this->ui.Name_LineEdit, &QLineEdit::textChanged, this, &DJctNodePanel::enableConfirmButton);
	QObject::connect(this->ui.Details_plainTextEdit, &QPlainTextEdit::textChanged, this, &DJctNodePanel::enableConfirmButton);
	QObject::connect(this->ui.Signalized_comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DJctNodePanel::enableConfirmButton);
	QObject::connect(this->ui.Box_Junction_comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DJctNodePanel::enableConfirmButton);

	//connect Function mode
	QObject::connect(this->jctModeButton, &QPushButton::clicked, function, &DJctEditFunction::cancelAndJctMode);
}

void DJctNodePanel::failMessage(QString&& msg) {
	this->ok_Buttons->ref_feedbackLabel->setText(msg);
	this->ok_Buttons->ref_feedbackLabel->update();
}

// --- --- --- --- --- Getters for relaying input form values --- --- --- --- ---

QString DJctNodePanel::getNameInput() {
	return this->ui.Name_LineEdit->text();
}

QString DJctNodePanel::getDetailsInput() {
	return this->ui.Details_plainTextEdit->toPlainText();
}

bool DJctNodePanel::getIsSigInput() {
	return this->ui.Signalized_comboBox->currentIndex();
}

bool DJctNodePanel::getIsBoxJctInput() {
	return this->ui.Box_Junction_comboBox->currentIndex();
}

// --- --- --- --- --- Private Utils --- --- --- --- ---

void DJctNodePanel::insertHintLabel(Type_JctPanel type) {
	this->hint_Label = new QLabel(this->ui.groupBox);
	this->hint_Label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	this->hint_Label->setWordWrap(true);

	switch (type) {
	case Type_JctPanel::create:
		this->hint_Label->setText(hintTextCreateJct.c_str());
		break;
	case Type_JctPanel::edit:
		this->hint_Label->setText(hintTextEditJct.c_str());
		break;
	}

	//always put the hint label at top
	int numOfWidget = this->ui.verticalLayout_2->count();
	this->ui.verticalLayout_2->insertWidget(0, this->hint_Label);
}

void DJctNodePanel::insertOkButtons() {
	this->ok_Buttons = new DOkCancelButtons(this->ui.groupBox);
	this->ok_Buttons->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

	//always put the hint label at bottom before the spacer
	int numOfWidget = this->ui.verticalLayout_2->count();
	this->ui.verticalLayout_2->insertWidget(numOfWidget - 1, this->ok_Buttons);
}

void DJctNodePanel::insertJctModeButton() {
	this->jctModeButton = new QPushButton(this->ui.groupBox);
	this->jctModeButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	this->jctModeButton->setText("Launch Junction Mode");

	//always put the hint label at bottom before the spacer
	int numOfWidget = this->ui.verticalLayout_2->count();
	this->ui.verticalLayout_2->insertWidget(numOfWidget - 1, this->jctModeButton);
}

void DJctNodePanel::showJctIdAndParam() {
	//show the arc id
	this->ui.IDText_Label->setText("Junction ID");
	this->ui.ID_Label->setText(std::to_string(this->ref_jctNode->getId()).c_str());
	//show name and details
	this->ui.Name_LineEdit->setText({ this->ref_jctNode->getVisInfo().getName().c_str() });
	this->ui.Details_plainTextEdit->setPlainText({ this->ref_jctNode->getVisInfo().getDetails().c_str() });
	//show the existing param, must restrict bool to valid index values
	this->ui.Signalized_comboBox->setCurrentIndex(this->ref_jctNode->getHasSig() ? 1 : 0);
	this->ui.Box_Junction_comboBox->setCurrentIndex(this->ref_jctNode->getHasYellowBox() ? 1 : 0);
}

void DJctNodePanel::showJctDefaultParam() {
	//TODO fetch the default params from somewhere
}


