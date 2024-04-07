#include "DSimSettingsPanel.h"

#include "Functions/Simulation/DRunSimFunction.h"
#include "PanelWidgets/DOkCancelButtons.h"
#include "Utils/MouseWheelWidgetAdjustmentGuard.h"

// --- --- --- --- --- Hint Texts --- --- --- --- ---

const std::string DSimSettingsPanel::hintText{
"A valid demand set and signal set must first be created in order to simulate\r\n\
\r\n"
};

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DSimSettingsPanel::DSimSettingsPanel(const QList<QString>& sigSetEntries, const QList<QString>& dmdSetEntries) : QWidget(nullptr) {
	ui.setupUi(this);

	//setup combo box
	this->ui.sigPlan_comboBox->addItems(sigSetEntries);
	this->ui.dmdPlan_comboBox->addItems(dmdSetEntries);

	//make sure time can input
	this->ui.time_spinBox->setMaximum(INT_MAX);
	// NO MORE SCROLLING
	auto srolling = new MouseWheelWidgetAdjustmentGuard{ this };
	this->ui.time_spinBox->installEventFilter(srolling);
	this->ui.sigPlan_comboBox->installEventFilter(srolling);
	this->ui.dmdPlan_comboBox->installEventFilter(srolling);

	//hint text
	this->hint_Label = new QLabel(this->ui.DCell_groupBox1);
	this->hint_Label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
	this->hint_Label->setWordWrap(true);
	this->hint_Label->setText(this->hintText.c_str());
	this->ui.verticalLayout_3->insertWidget(0, this->hint_Label);

#ifdef LOCKED_GUI_DEPLOY
	//1800 simtime limit
	this->ui.time_spinBox->setMaximum(1800);
#endif

	//ok button
	this->ok_Buttons = new DOkCancelButtons(this->ui.DCell_groupBox1);
	this->ok_Buttons->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
	//put it under the param input, the result layout still has spacing
	this->ui.verticalLayout_3->insertWidget(2, this->ok_Buttons);

	//results stuff
	this->showResults(false);
}

DSimSettingsPanel::~DSimSettingsPanel() {
}

// --- --- --- --- --- Settings for Function --- --- --- --- ---

void DSimSettingsPanel::setupButtons(DRunSimFunction* function) {
	QObject::connect(this->ok_Buttons->ref_confirmButton, &QPushButton::clicked, function, &DRunSimFunction::checkAndRunSim);
	QObject::connect(this->ok_Buttons->ref_cancelButton, &QPushButton::clicked, function, &DRunSimFunction::cancel);
	QObject::connect(this->ui.close_pushButton, &QPushButton::clicked, function, &DRunSimFunction::cancel);
	QObject::connect(this->ui.playBack_pushButton, &QPushButton::clicked, function, &DRunSimFunction::closeAndOpenRecView);
}

void DSimSettingsPanel::failMessage(QString&& msg) {
	this->ok_Buttons->ref_feedbackLabel->setText(msg);
	this->ok_Buttons->ref_feedbackLabel->update();
}

void DSimSettingsPanel::showResults(bool isShow, double tolDelay, double tolDemand, double avgDelay) {
	this->ui.TolDelayText_Label->setVisible(isShow);
	this->ui.TolDemandText_Label->setVisible(isShow);
	this->ui.AvgDelayText_Label->setVisible(isShow);

	this->ui.TolDelay_doubleSpinBox->setVisible(isShow);
	this->ui.TolDemand_doubleSpinBox->setVisible(isShow);
	this->ui.AvgDelay_doubleSpinBox->setVisible(isShow);

	//the close buttons
	this->ui.playBack_pushButton->setVisible(isShow);
	this->ui.playBack_pushButton->setEnabled(isShow);
	this->ui.close_pushButton->setVisible(isShow);
	this->ui.close_pushButton->setEnabled(isShow);

	if (isShow) {
		//read only the input params
		this->ui.sigPlan_comboBox->setEnabled(false);
		this->ui.dmdPlan_comboBox->setEnabled(false);
		this->ui.time_spinBox->setReadOnly(true);
		this->ui.name_lineEdit->setReadOnly(true);
		this->ui.Details_PlainTextEdit->setReadOnly(true);

		//can't be bothered to find hte double max or qreal max -JLo
		this->ui.TolDelay_doubleSpinBox->setMaximum(tolDelay + 1);
		this->ui.TolDemand_doubleSpinBox->setMaximum(tolDemand + 1);
		this->ui.AvgDelay_doubleSpinBox->setMaximum(avgDelay + 1);
		this->ui.TolDelay_doubleSpinBox->setValue(tolDelay);
		this->ui.TolDemand_doubleSpinBox->setValue(tolDemand);
		this->ui.AvgDelay_doubleSpinBox->setValue(avgDelay);

		//remove ok canel button
		this->ui.verticalLayout_3->removeWidget(this->ok_Buttons);
		delete this->ok_Buttons;
	}
}

// --- --- --- --- --- Getters for relaying input form values --- --- --- --- ---

QString DSimSettingsPanel::getSigSetInput() {
	return this->ui.sigPlan_comboBox->currentText();
}

QString DSimSettingsPanel::getDmdSetInput() {
	return this->ui.dmdPlan_comboBox->currentText();
}

int DSimSettingsPanel::getTimeInput() {
	return this->ui.time_spinBox->value();
}

QString DSimSettingsPanel::getNameInput() {
	return this->ui.name_lineEdit->text();
}

QString DSimSettingsPanel::getDetailsInput() {
	return this->ui.Details_PlainTextEdit->toPlainText();
}

// --- --- --- --- --- Setters for default settings --- --- --- --- ---

void DSimSettingsPanel::setSigSetInput(int index) {
	this->ui.sigPlan_comboBox->setCurrentIndex(index);
}

void DSimSettingsPanel::setDmdSetInput(int index) {
	this->ui.dmdPlan_comboBox->setCurrentIndex(index);
}

void DSimSettingsPanel::setTimeInput(int time) {
	this->ui.time_spinBox->setValue(time);
}
