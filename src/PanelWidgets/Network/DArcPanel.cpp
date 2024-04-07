#include "DArcPanel.h"

#include <string>

#include <Arc.h>
#include <Jct_Link.h>

#include "DOkCancelButtons.h"
#include "Functions/Network/DArcCreateFunction.h"
#include "Functions/Network/DArcEditFunction.h"
#include "Functions/Junction/DJctLinkCreateFunction.h"
#include "Functions/Junction/DJctLinkEditFunction.h"
#include "Utils/MouseWheelWidgetAdjustmentGuard.h"
#include "Utils/DDefaultParamData.h"

// --- --- --- --- --- Hint Texts --- --- --- --- ---

std::string hintTextCreateArc{
"Single click to select the start point of an Arc\r\n\
Double click to select the end point of an Arc\r\n\
Drag the yellow dots to move the spine points of the Arc\r\n\
Control right click along the arc to add a spine point\r\n\
Select and press delete to delete a spine point\r\n\
Use the recalculate button to update the arc length, or enter one manually\r\n"
};

std::string hintTextEditArc{
"Drag the yellow dots to move the spine points of the Arc\r\n\
Control right click along the arc to add a spine point\r\n\
Select and press delete to delete a spine point\r\n\
Use the recalculate button to update the arc length, or enter one manually\r\n"
};

std::string hintTextCreateJL{
"Click on an entering cell to select the from cell\r\n\
Click on an exit cell to select the end cell\r\n\
Use the recalculate button to estimate the Junction link length, \r\n\
however entering one manually is more accurate\r\n"
};

std::string hintTextEditJL{
"Use the recalculate button to estimate the Junction link length, \r\n\
however entering one manually is more accurate\r\n"
};

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DArcPanel::DArcPanel(Type_ArcPanel type, const DISCO2_API::Arc* arc, DefaultParamDataSet* dparam) :
	QWidget(nullptr), ref_arc(arc), ref_dParam(dparam) {

	ui.setupUi(this);
	this->ui.SaturationFlow_doubleSpinBox->setMaximum(DBL_MAX);
	this->ui.JamDensity_doubleSpinBox->setMaximum(DBL_MAX);
	// NO MORE SCROLLING
	MouseWheelWidgetAdjustmentGuard* noMoreScrolling = new MouseWheelWidgetAdjustmentGuard{ this };
	this->ui.Length_doubleSpinBox->installEventFilter(noMoreScrolling);
	this->ui.FreeFlowSpeedLabel_doubleSpinBox->installEventFilter(noMoreScrolling);
	this->ui.BackwardShockwaveSpeed_doubleSpinBox->installEventFilter(noMoreScrolling);
	this->ui.JamDensity_doubleSpinBox->installEventFilter(noMoreScrolling);
	this->ui.SaturationFlow_doubleSpinBox->installEventFilter(noMoreScrolling);
	this->ui.NumberOfLane_spinBox->installEventFilter(noMoreScrolling);

	switch (type) {
	case Type_ArcPanel::create:
		ui.DArc_groupBox1->setTitle("Create Arc");
		this->insertHintLabel(type);
		this->showArcDefaultParam();
		this->insertOkButtons();
		this->setEditable(false);	//lock input initialy
		break;
	case Type_ArcPanel::edit:
		ui.DArc_groupBox1->setTitle("Edit Arc");
		this->insertHintLabel(type);
		this->showArcIdAndParam();
		this->insertOkButtons();
		this->setEditable(true);
		//need to override editable
		this->ui.NumberOfLane_spinBox->setReadOnly(true);
		this->ui.NumberOfLane_spinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
		break;
	case Type_ArcPanel::view:
		ui.DArc_groupBox1->setTitle("Arc");
		this->showArcIdAndParam();
		this->setEditable(false);
		break;
	
	case Type_ArcPanel::createJL:
		this->ref_jctLink = dynamic_cast<const DISCO2_API::Jct_Link*>(this->ref_arc);
		ui.DArc_groupBox1->setTitle("Create Junction Link");
		this->insertHintLabel(type);
		this->showJctLinkDefaultParam();
		this->insertOkButtons();
		this->setEditable(false);	//lock input initialy
		break;
	case Type_ArcPanel::editJL:
		this->ref_jctLink = dynamic_cast<const DISCO2_API::Jct_Link*>(this->ref_arc);
		ui.DArc_groupBox1->setTitle("Edit Junction Link");
		this->insertHintLabel(type);
		this->showJctLinkIdAndParam();
		this->insertOkButtons();
		this->setEditable(true);
		break;
	case Type_ArcPanel::viewJL:
		this->ref_jctLink = dynamic_cast<const DISCO2_API::Jct_Link*>(this->ref_arc);
		ui.DArc_groupBox1->setTitle("Junction Link");
		this->showJctLinkIdAndParam();
		this->setEditable(false);
		break;
	}
}

DArcPanel::~DArcPanel() {

}

// --- --- --- --- --- Settings for Functions --- --- --- --- ---

void DArcPanel::setEditable(bool editable) {
	this->ui.recalLength_pushButton->setVisible(editable);
	//set all to read only mode
	this->ui.Name_LineEdit->setReadOnly(!editable);
	this->ui.Details_plainTextEdit->setReadOnly(!editable);
	this->ui.BackwardShockwaveSpeed_doubleSpinBox->setReadOnly(!editable);
	this->ui.FreeFlowSpeedLabel_doubleSpinBox->setReadOnly(!editable);
	this->ui.JamDensity_doubleSpinBox->setReadOnly(!editable);
	this->ui.Length_doubleSpinBox->setReadOnly(!editable);
	this->ui.SaturationFlow_doubleSpinBox->setReadOnly(!editable);
	//only editable iff create arc
	if (editable && this->ref_jctLink == nullptr && this->ref_arc == nullptr) {
		this->ui.NumberOfLane_spinBox->setReadOnly(false);
		this->ui.NumberOfLane_spinBox->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
	}
	else {
		this->ui.NumberOfLane_spinBox->setReadOnly(true);
		this->ui.NumberOfLane_spinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
		if (editable) {
			//change the color to show params can't be edited
			QPalette palette = this->ui.NumberOfLane_spinBox->palette();
			palette.setColor(QPalette::ColorRole::Base, this->palette().color(QPalette::ColorRole::Background));
			this->ui.NumberOfLane_spinBox->setPalette(palette);
		}
	}
}

void DArcPanel::enableConfirmButton() {
	this->ok_Buttons->ref_confirmButton->setEnabled(true);
}

void DArcPanel::setupCreateButtons(DArcCreateFunction* function) {
	//set the text of the buttons
	this->ok_Buttons->ref_confirmButton->setText("Create");
	this->ok_Buttons->ref_cancelButton->setText("Cancel");
	this->ok_Buttons->ref_confirmButton->setEnabled(false);

	//connect buttons to function methods
	QObject::connect(this->ok_Buttons->ref_confirmButton, &QPushButton::clicked, function, &DArcCreateFunction::checkAndCreateArc);
	QObject::connect(this->ok_Buttons->ref_cancelButton, &QPushButton::clicked, function, &DArcCreateFunction::cancel);
	QObject::connect(this->ui.recalLength_pushButton, &QPushButton::clicked, function, &DArcCreateFunction::recalLength);
}

void DArcPanel::setupEditButtons(DArcEditFunction* function) {
	//set the text of the buttons
	this->ok_Buttons->ref_confirmButton->setText("Edit");
	this->ok_Buttons->ref_cancelButton->setText("Cancel");
	this->ok_Buttons->ref_confirmButton->setEnabled(false);

	//connect buttons to function methods
	QObject::connect(this->ok_Buttons->ref_confirmButton, &QPushButton::clicked, function, &DArcEditFunction::checkAndEditArc);
	QObject::connect(this->ok_Buttons->ref_cancelButton, &QPushButton::clicked, function, &DArcEditFunction::cancel);
	QObject::connect(this->ui.recalLength_pushButton, &QPushButton::clicked, function, &DArcEditFunction::recalLength);

	//connect internal value changes to the confirm button
	QObject::connect(this->ui.Name_LineEdit, &QLineEdit::textChanged, this, &DArcPanel::enableConfirmButton);
	QObject::connect(this->ui.Details_plainTextEdit, &QPlainTextEdit::textChanged, this, &DArcPanel::enableConfirmButton);
	QObject::connect(this->ui.BackwardShockwaveSpeed_doubleSpinBox,  QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DArcPanel::enableConfirmButton);
	QObject::connect(this->ui.FreeFlowSpeedLabel_doubleSpinBox,  QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DArcPanel::enableConfirmButton);
	QObject::connect(this->ui.JamDensity_doubleSpinBox,  QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DArcPanel::enableConfirmButton);
	QObject::connect(this->ui.Length_doubleSpinBox,  QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DArcPanel::enableConfirmButton);
	QObject::connect(this->ui.SaturationFlow_doubleSpinBox,  QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DArcPanel::enableConfirmButton);
}

void DArcPanel::setupCreateJLButtons(DJctLinkCreateFunction* function) {
	//set the text of the buttons
	this->ok_Buttons->ref_confirmButton->setText("Create");
	this->ok_Buttons->ref_cancelButton->setText("Cancel");
	this->ok_Buttons->ref_confirmButton->setEnabled(false);

	//connect buttons to function methods
	QObject::connect(this->ok_Buttons->ref_confirmButton, &QPushButton::clicked, function, &DJctLinkCreateFunction::checkAndCreateJLink);
	QObject::connect(this->ok_Buttons->ref_cancelButton, &QPushButton::clicked, function, &DJctLinkCreateFunction::cancel);
	QObject::connect(this->ui.recalLength_pushButton, &QPushButton::clicked, function, &DJctLinkCreateFunction::recalLength);
}

void DArcPanel::setupEditJLButtons(DJctLinkEditFunction* function) {
	//set the text of the buttons
	this->ok_Buttons->ref_confirmButton->setText("Edit");
	this->ok_Buttons->ref_cancelButton->setText("Cancel");
	this->ok_Buttons->ref_confirmButton->setEnabled(false);

	//connect buttons to function methods
	QObject::connect(this->ok_Buttons->ref_confirmButton, &QPushButton::clicked, function, &DJctLinkEditFunction::checkAndEditJctLink);
	QObject::connect(this->ok_Buttons->ref_cancelButton, &QPushButton::clicked, function, &DJctLinkEditFunction::cancel);
	QObject::connect(this->ui.recalLength_pushButton, &QPushButton::clicked, function, &DJctLinkEditFunction::recalLength);

	//connect internal value changes to the confirm button
	QObject::connect(this->ui.Name_LineEdit, &QLineEdit::textChanged, this, &DArcPanel::enableConfirmButton);
	QObject::connect(this->ui.Details_plainTextEdit, &QPlainTextEdit::textChanged, this, &DArcPanel::enableConfirmButton);
	QObject::connect(this->ui.BackwardShockwaveSpeed_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DArcPanel::enableConfirmButton);
	QObject::connect(this->ui.FreeFlowSpeedLabel_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DArcPanel::enableConfirmButton);
	QObject::connect(this->ui.JamDensity_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DArcPanel::enableConfirmButton);
	QObject::connect(this->ui.Length_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DArcPanel::enableConfirmButton);
	QObject::connect(this->ui.SaturationFlow_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DArcPanel::enableConfirmButton);
}

void DArcPanel::failMessage(QString&& msg) {
	this->ok_Buttons->ref_feedbackLabel->setText(msg);
	this->ok_Buttons->ref_feedbackLabel->update();
}

void DArcPanel::setLengthValue(double length) {
	this->ui.Length_doubleSpinBox->setValue(length);
	this->ui.Length_doubleSpinBox->update();
}

// --- --- --- --- --- Getters for relaying input form values --- --- --- --- ---

double DArcPanel::getQInput() {
	return this->ui.SaturationFlow_doubleSpinBox->value()/3600;
}

double DArcPanel::getKInput() {
	return this->ui.JamDensity_doubleSpinBox->value()/1000;
}

double DArcPanel::getWInput() {
	return this->ui.BackwardShockwaveSpeed_doubleSpinBox->value();
}

double DArcPanel::getVfInput() {
	return this->ui.FreeFlowSpeedLabel_doubleSpinBox->value();
}

double DArcPanel::getLengthInput() {
	return this->ui.Length_doubleSpinBox->value();
}

int DArcPanel::getNumOfLanesInput() {
	return this->ui.NumberOfLane_spinBox->value();
}

QString DArcPanel::getNameInput() {
	return this->ui.Name_LineEdit->text();
}

QString DArcPanel::getDetailsInput() {
	return this->ui.Details_plainTextEdit->toPlainText();
}

// --- --- --- --- --- Private Utils --- --- --- --- ---

void DArcPanel::insertHintLabel(Type_ArcPanel type) {
	
	this->hint_Label = new QLabel(this->ui.DArc_groupBox1);
	this->hint_Label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	this->hint_Label->setWordWrap(true);

	switch (type) {
	case Type_ArcPanel::create:
		this->hint_Label->setText(hintTextCreateArc.c_str());
		break;
	case Type_ArcPanel::edit:
		this->hint_Label->setText(hintTextEditArc.c_str());
		break;
	case Type_ArcPanel::createJL:
		this->hint_Label->setText(hintTextCreateJL.c_str());
		break;
	case Type_ArcPanel::editJL:
		this->hint_Label->setText(hintTextEditJL.c_str());
		break;
	}
	
	//always put the hint label at top
	int numOfWidget = this->ui.verticalLayout->count();
	this->ui.verticalLayout->insertWidget(0, this->hint_Label);

}

void DArcPanel::insertOkButtons() {
	this->ok_Buttons = new DOkCancelButtons(this->ui.DArc_groupBox1);
	this->ok_Buttons->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

	//always put the hint label at bottom before the spacer
	int numOfWidget = this->ui.verticalLayout->count();
	this->ui.verticalLayout->insertWidget(numOfWidget-1, this->ok_Buttons);
}

void DArcPanel::showArcIdAndParam() {
	//show the arc id
	this->ui.ArcIdText_Label->setText("Arc ID");
	this->ui.ArcId_Label->setText(std::to_string(this->ref_arc->getId()).c_str());
	//show name and details
	this->ui.Name_LineEdit->setText({ this->ref_arc->getVisInfo().getName().c_str() });
	this->ui.Details_plainTextEdit->setPlainText({ this->ref_arc->getVisInfo().getDetails().c_str() });
	//show the existing param
	this->ui.Length_doubleSpinBox->setValue(this->ref_arc->getLength());
	this->ui.NumberOfLane_spinBox->setValue(this->ref_arc->getNumOfLanes());
	this->ui.SaturationFlow_doubleSpinBox->setValue(this->ref_arc->getQ()*3600);
	this->ui.JamDensity_doubleSpinBox->setValue(this->ref_arc->getK()*1000);
	this->ui.FreeFlowSpeedLabel_doubleSpinBox->setValue(this->ref_arc->getVf());
	this->ui.BackwardShockwaveSpeed_doubleSpinBox->setValue(this->ref_arc->getW());
	//disable the number of lanes edit
	this->ui.NumberOfLane_spinBox->setReadOnly(true);
	this->ui.NumberOfLane_spinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
}

void DArcPanel::showArcDefaultParam() {
	//fetch the default params from somewhere
	// empty pointer of DefaultParamDataSet would not set any values into the panel
	if (!this->ref_dParam) {
		return;
	}

	// save newly amended inputted params to default data struct bakcup
	if (this->ref_dParam->isShowLastInput && this->ref_dParam->m_ArcFlowParams.FreeFlowSpeed != -1) {
		// true, show arcflow param
		this->ui.NumberOfLane_spinBox->setValue(ref_dParam->m_ArcFlowParams.NumberOfLane);
		this->ui.FreeFlowSpeedLabel_doubleSpinBox->setValue(ref_dParam->m_ArcFlowParams.FreeFlowSpeed);
		this->ui.BackwardShockwaveSpeed_doubleSpinBox->setValue(ref_dParam->m_ArcFlowParams.BackwardShockwaveSpeed);
		this->ui.JamDensity_doubleSpinBox->setValue(ref_dParam->m_ArcFlowParams.JamDensity*1000);
		this->ui.SaturationFlow_doubleSpinBox->setValue(ref_dParam->m_ArcFlowParams.SaturationFlow*3600);
	}
	else {
		// show flow param
		this->ui.NumberOfLane_spinBox->setValue(ref_dParam->m_FlowParams.NumberOfLane);
		this->ui.FreeFlowSpeedLabel_doubleSpinBox->setValue(ref_dParam->m_FlowParams.FreeFlowSpeed);
		this->ui.BackwardShockwaveSpeed_doubleSpinBox->setValue(ref_dParam->m_FlowParams.BackwardShockwaveSpeed);
		this->ui.JamDensity_doubleSpinBox->setValue(ref_dParam->m_FlowParams.JamDensity*1000);
		this->ui.SaturationFlow_doubleSpinBox->setValue(ref_dParam->m_FlowParams.SaturationFlow*3600);
	}
}

void DArcPanel::showJctLinkIdAndParam() {
	//show the arc id
	this->ui.ArcIdText_Label->setText("Junction Link ID");
	this->ui.ArcId_Label->setText(std::to_string(this->ref_jctLink->getJctLinkId()).c_str());
	//show name and details
	this->ui.Name_LineEdit->setText({ this->ref_arc->getVisInfo().getName().c_str() });
	this->ui.Details_plainTextEdit->setPlainText({ this->ref_arc->getVisInfo().getDetails().c_str() });
	//show the existing param
	this->ui.Length_doubleSpinBox->setValue(this->ref_arc->getLength());
	this->ui.NumberOfLane_spinBox->setValue(this->ref_arc->getNumOfLanes());
	this->ui.SaturationFlow_doubleSpinBox->setValue(this->ref_arc->getQ()*3600);
	this->ui.JamDensity_doubleSpinBox->setValue(this->ref_arc->getK()*1000);
	this->ui.FreeFlowSpeedLabel_doubleSpinBox->setValue(this->ref_arc->getVf());
	this->ui.BackwardShockwaveSpeed_doubleSpinBox->setValue(this->ref_arc->getW());
	//disable the number of lanes edit
	this->ui.NumberOfLane_spinBox->setReadOnly(true);
	this->ui.NumberOfLane_spinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
}

void DArcPanel::showJctLinkDefaultParam() {
	//hide the num of lane stuff
	this->ui.NumberOfLane_Label->setVisible(false);
	this->ui.NumberOfLane_spinBox->setVisible(false);

	//fetch the default params from somewhere
	// empty pointer of DefaultParamDataSet would not set any values into the panel
	if (!ref_dParam) {
		return;
	}

	// save newly amended inputted params to default data struct bakcup
	if (this->ref_dParam->isShowLastInput && this->ref_dParam->m_JLinkFlowParams.FreeFlowSpeed != -1) {
		// true, show arcflow param
		this->ui.NumberOfLane_spinBox->setValue(ref_dParam->m_JLinkFlowParams.NumberOfLane);
		this->ui.FreeFlowSpeedLabel_doubleSpinBox->setValue(ref_dParam->m_JLinkFlowParams.FreeFlowSpeed);
		this->ui.BackwardShockwaveSpeed_doubleSpinBox->setValue(ref_dParam->m_JLinkFlowParams.BackwardShockwaveSpeed);
		this->ui.JamDensity_doubleSpinBox->setValue(ref_dParam->m_JLinkFlowParams.JamDensity*1000);
		this->ui.SaturationFlow_doubleSpinBox->setValue(ref_dParam->m_JLinkFlowParams.SaturationFlow*3600);
	}
	else {
		// show flow param
		this->ui.NumberOfLane_spinBox->setValue(ref_dParam->m_FlowParams.NumberOfLane);
		this->ui.FreeFlowSpeedLabel_doubleSpinBox->setValue(ref_dParam->m_FlowParams.FreeFlowSpeed);
		this->ui.BackwardShockwaveSpeed_doubleSpinBox->setValue(ref_dParam->m_FlowParams.BackwardShockwaveSpeed);
		this->ui.JamDensity_doubleSpinBox->setValue(ref_dParam->m_FlowParams.JamDensity*1000);
		this->ui.SaturationFlow_doubleSpinBox->setValue(ref_dParam->m_FlowParams.SaturationFlow*3600);
	}
}


