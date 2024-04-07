#include "DGASettingsPanel.h"

#include "PanelWidgets/DOkCancelButtons.h"
#include "Utils/MouseWheelWidgetAdjustmentGuard.h"
#include "Functions/Simulation/DRunGAFunction.h"

// --- --- --- --- --- Hint Texts --- --- --- --- ---

const std::string DGASettingsPanel::hintText{
"A valid demand set and signal plan must first be created in order to simulate\r\n\
\r\n\
Optimised signal plans are NOT saved automatically,\r\n\
to save the optimised signal plans, click the save button before closing\r\n\
\r\n"
};

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DGASettingsPanel::DGASettingsPanel(const QList<QString>& sigSetEntries, const QList<QString>& dmdSetEntries) : QWidget(nullptr) {
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
	this->ui.genNum_spinBox->installEventFilter(srolling);
	this->ui.subGenNum_spinBox->installEventFilter(srolling);
	this->ui.popSize_spinBox->installEventFilter(srolling);
	this->ui.eliteRate_doubleSpinBox->installEventFilter(srolling);
	this->ui.crossoverRate_doubleSpinBox->installEventFilter(srolling);
	this->ui.mutationRate_doubleSpinBox->installEventFilter(srolling);
	this->ui.powerFactor_doubleSpinBox->installEventFilter(srolling);
	this->ui.optCyc_comboBox->installEventFilter(srolling);
	this->ui.optOffset_comboBox->installEventFilter(srolling);
	this->ui.optGreen_comboBox->installEventFilter(srolling);
	this->ui.matchCyc_comboBox->installEventFilter(srolling);

#ifdef LOCKED_GUI_DEPLOY
	//1800 simtime limit
	this->ui.time_spinBox->setMaximum(1800);
#endif

	//hint text
	this->ui.hint_label->setWordWrap(true);
	this->ui.hint_label->setText(this->hintText.c_str());

	//ok button
	this->ok_Buttons = new DOkCancelButtons(this->ui.scrollArea);
	this->ok_Buttons->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
	//put it under the param input, the result layout still has spacing
	this->ui.verticalLayout->insertWidget(1, this->ok_Buttons);

	//results stuff
	this->showResults(false);
}

DGASettingsPanel::~DGASettingsPanel() {
}

// --- --- --- --- --- Settings for Function --- --- --- --- ---

void DGASettingsPanel::setupButtons(DRunGAFunction* function) {
	QObject::connect(this->ok_Buttons->ref_confirmButton, &QPushButton::clicked, function, &DRunGAFunction::checkAndRunGA);
	QObject::connect(this->ok_Buttons->ref_cancelButton, &QPushButton::clicked, function, &DRunGAFunction::cancel);
	QObject::connect(this->ui.close_pushButton, &QPushButton::clicked, function, &DRunGAFunction::cancel);
	QObject::connect(this->ui.saveSig_pushButton, &QPushButton::clicked, function, &DRunGAFunction::saveSigSet);
	QObject::connect(this->ui.playBack_pushButton, &QPushButton::clicked, function, &DRunGAFunction::closeAndOpenRecView);
}

void DGASettingsPanel::failMessage(QString&& msg) {
	this->ok_Buttons->ref_feedbackLabel->setText(msg);
	this->ok_Buttons->ref_feedbackLabel->update();
}

void DGASettingsPanel::showResults(bool isShow, double tolDelay, double tolDemand, double avgDelay) {
	this->ui.saveSig_groupBox->setVisible(isShow);
	this->ui.results_groupBox->setVisible(isShow);

	if (isShow) {
		//read only the input params
		//DESIGN restrict combobox choice rather than disabling it -JLo
		this->ui.sigPlan_comboBox->setEnabled(false);
		this->ui.dmdPlan_comboBox->setEnabled(false);
		this->ui.time_spinBox->setReadOnly(true);
		this->ui.name_lineEdit->setReadOnly(true);
		this->ui.Details_PlainTextEdit->setReadOnly(true);
		this->ui.genNum_spinBox->setReadOnly(true);
		this->ui.subGenNum_spinBox->setReadOnly(true);
		this->ui.popSize_spinBox->setReadOnly(true);
		this->ui.eliteRate_doubleSpinBox->setReadOnly(true);
		this->ui.crossoverRate_doubleSpinBox->setReadOnly(true);
		this->ui.mutationRate_doubleSpinBox->setReadOnly(true);
		this->ui.powerFactor_doubleSpinBox->setReadOnly(true);
		this->ui.optCyc_comboBox->setEnabled(false);
		this->ui.optOffset_comboBox->setEnabled(false);
		this->ui.optGreen_comboBox->setEnabled(false);
		this->ui.matchCyc_comboBox->setEnabled(false);

		//can't be bothered to find hte double max or qreal max -JLo
		this->ui.TolDelay_doubleSpinBox->setMaximum(tolDelay + 1);
		this->ui.TolDemand_doubleSpinBox->setMaximum(tolDemand + 1);
		this->ui.AvgDelay_doubleSpinBox->setMaximum(avgDelay + 1);
		this->ui.TolDelay_doubleSpinBox->setValue(tolDelay);
		this->ui.TolDemand_doubleSpinBox->setValue(tolDemand);
		this->ui.AvgDelay_doubleSpinBox->setValue(avgDelay);

		//remove ok canel button
		this->ok_Buttons->setVisible(false);
	}
}

void DGASettingsPanel::sigFailMessage(QString&& msg) {
	this->ui.saveSig_label->setText(msg);
}

void DGASettingsPanel::sigSaved(int savedSigSetId) {
	//do text
	std::string t{ "Signal Set Saved to SigSetId: " + std::to_string(savedSigSetId) };
	this->ui.saveSig_label->setText(t.c_str());
	//lock things
	this->ui.saveSig_pushButton->setEnabled(false);
	this->ui.sigName_lineEdit->setReadOnly(true);
	this->ui.sigDetails_PlainTextEdit->setReadOnly(true);
}

// --- --- --- --- --- Getters for relaying input form values --- --- --- --- ---

QString DGASettingsPanel::getSigSetInput() {
	return this->ui.sigPlan_comboBox->currentText();
}

QString DGASettingsPanel::getDmdSetInput() {
	return this->ui.dmdPlan_comboBox->currentText();
}

int DGASettingsPanel::getTimeInput() {
	return this->ui.time_spinBox->value();
}

QString DGASettingsPanel::getNameInput() {
	return this->ui.name_lineEdit->text();
}

QString DGASettingsPanel::getDetailsInput() {
	return this->ui.Details_PlainTextEdit->toPlainText();
}

int DGASettingsPanel::getGAType() {
	return this->ui.gaType_comboBox->currentIndex();
}

int DGASettingsPanel::getNumOfThreads() {
	return this->ui.threadNum_spinBox->value();
}

int DGASettingsPanel::getNumOfGen() {
	return this->ui.genNum_spinBox->value();
}

int DGASettingsPanel::getNumOfSubGen() {
	return this->ui.subGenNum_spinBox->value();
}

int DGASettingsPanel::getPopSize() {
	return this->ui.popSize_spinBox->value();
}

double DGASettingsPanel::getEliteRate() {
	return this->ui.eliteRate_doubleSpinBox->value();
}

double DGASettingsPanel::getCrossoverRate() {
	return this->ui.crossoverRate_doubleSpinBox->value();
}

double DGASettingsPanel::getMutationRate() {
	return this->ui.mutationRate_doubleSpinBox->value();
}

double DGASettingsPanel::getPowerFactor() {
	return this->ui.powerFactor_doubleSpinBox->value();
}

bool DGASettingsPanel::getOptCyc() {
	return this->ui.optCyc_comboBox->currentIndex();
}

bool DGASettingsPanel::getOptOffset() {
	return this->ui.optOffset_comboBox->currentIndex();
}

bool DGASettingsPanel::getOptGreen() {
	return this->ui.optGreen_comboBox->currentIndex();
}

bool DGASettingsPanel::getMatchCyc() {
	return this->ui.matchCyc_comboBox->currentIndex();
}

QString DGASettingsPanel::getSigNameInput() {
	return this->ui.sigName_lineEdit->text();
}

QString DGASettingsPanel::getSigDetailsInput() {
	return this->ui.sigDetails_PlainTextEdit->toPlainText();
}

// --- --- --- --- --- Setters for Default Param --- --- --- --- ---

void DGASettingsPanel::setSigSetInput(int index) {
	this->ui.sigPlan_comboBox->setCurrentIndex(index);
}

void DGASettingsPanel::setDmdSetInput(int index) {
	this->ui.dmdPlan_comboBox->setCurrentIndex(index);
}

void DGASettingsPanel::setTimeInput(int time) {
	this->ui.time_spinBox->setValue(time);
}

void DGASettingsPanel::setGAType(int index) {
	this->ui.gaType_comboBox->setCurrentIndex(index);
}

void DGASettingsPanel::setNumOfThreads(int numOfThreads) {
	this->ui.threadNum_spinBox->setValue(numOfThreads);
}

void DGASettingsPanel::setNumOfGen(int numOfGen) {
	this->ui.genNum_spinBox->setValue(numOfGen);
}

void DGASettingsPanel::setNumOfSubGen(int numOfSubGen) {
	this->ui.subGenNum_spinBox->setValue(numOfSubGen);
}

void DGASettingsPanel::setPopSize(int popSize) {
	this->ui.popSize_spinBox->setValue(popSize);
}

void DGASettingsPanel::setEliteRate(double rate) {
	this->ui.eliteRate_doubleSpinBox->setValue(rate);
}

void DGASettingsPanel::setCrossoverRate(double rate) {
	this->ui.crossoverRate_doubleSpinBox->setValue(rate);
}

void DGASettingsPanel::setMutationRate(double rate) {
	this->ui.mutationRate_doubleSpinBox->setValue(rate);
}

void DGASettingsPanel::setPowerFactor(double powerFactor) {
	this->ui.powerFactor_doubleSpinBox->setValue(powerFactor);
}

void DGASettingsPanel::setOptCyc(bool isOptCycle) {
	this->ui.optCyc_comboBox->setCurrentIndex(isOptCycle ? 1 : 0);
}

void DGASettingsPanel::setOptOffset(bool isOptOffset) {
	this->ui.optOffset_comboBox->setCurrentIndex(isOptOffset ? 1 : 0);
}

void DGASettingsPanel::setOptGreen(bool isOptGreen) {
	this->ui.optGreen_comboBox->setCurrentIndex(isOptGreen ? 1 : 0);
}

void DGASettingsPanel::setMatchCyc(bool isMatchCyc) {
	this->ui.matchCyc_comboBox->setCurrentIndex(isMatchCyc ? 1 : 0);
}
