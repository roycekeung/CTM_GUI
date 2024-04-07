#include "DDefaultParamSettingDialogue.h"


//DISCO GUI stuff
#include "DOkCancelButtons.h"
#include "Utils/MouseWheelWidgetAdjustmentGuard.h"
#include "../Functions/Home/DDefaultParamSettingFunction.h"
#include "../Utils/DDefaultParamData.h"

//Qt lib
#include <QString>
#include <QtWidgets/QLabel>

//DISCO Core lib
#include "Editor_Sig.h"
#include "Sig_Container.h"
#include "Net_Container.h"


// --- --- --- --- --- Private Utils --- --- --- --- ---
// insert external button, text
void DDefaultParamSettingDialogue::insertOkButtons() {
	this->ok_Buttons = new DOkCancelButtons(this);
	this->ok_Buttons->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	//always put the okbutton at bottom after the spacer
	int numOfWidget = this->ui.verticalLayout->count();
	this->ui.verticalLayout->insertWidget(numOfWidget, this->ok_Buttons);
}

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---
DDefaultParamSettingDialogue::DDefaultParamSettingDialogue(const QList<QString>& sigSetEntries, const QList<QString>& dmdSetEntries,
	DefaultParamDataSet* dparam)
	: QDialog(nullptr) {
	ui.setupUi(this);

	//make sure SimRunTime and GARunTime can input
	this->ui.time_Sim_spinBox->setMaximum(INT_MAX);
	this->ui.time_GA_spinBox->setMaximum(INT_MAX);
	this->ui.SaturationFlow_doubleSpinBox->setMaximum(DBL_MAX);
	this->ui.JamDensity_doubleSpinBox->setMaximum(DBL_MAX);

	// --- --- --- --- --- NO MORE SCROLLING --- --- --- --- --- 
	MouseWheelWidgetAdjustmentGuard * scrolling = new MouseWheelWidgetAdjustmentGuard{ this };

	// Default Setting for Flow Parameters
	this->ui.NumberOfLane_spinBox->installEventFilter(scrolling);
	this->ui.FreeFlowSpeedLabel_doubleSpinBox->installEventFilter(scrolling);
	this->ui.BackwardShockwaveSpeed_doubleSpinBox->installEventFilter(scrolling);
	this->ui.JamDensity_doubleSpinBox->installEventFilter(scrolling);
	this->ui.SaturationFlow_doubleSpinBox->installEventFilter(scrolling);
	this->ui.isShowLastInput_comboBox->installEventFilter(scrolling);

	// Default Setting for Sim
	this->ui.sigPlan_Sim_comboBox->installEventFilter(scrolling);
	this->ui.dmdPlan_Sim_comboBox->installEventFilter(scrolling);
	this->ui.time_Sim_spinBox->installEventFilter(scrolling);
	//setup sim  Set ID Plan combo box
	this->ui.sigPlan_Sim_comboBox->addItems(sigSetEntries);
	this->ui.dmdPlan_Sim_comboBox->addItems(dmdSetEntries);

	// Default Setting for GA
	this->ui.sigPlan_GA_comboBox->installEventFilter(scrolling);
	this->ui.dmdPlan_GA_comboBox->installEventFilter(scrolling);
	this->ui.time_GA_spinBox->installEventFilter(scrolling);

	this->ui.gaType_GA_comboBox->installEventFilter(scrolling);
	this->ui.threadNum_spinBox->installEventFilter(scrolling);
	this->ui.genNum_GA_spinBox->installEventFilter(scrolling);
	this->ui.subGenNum_GA_spinBox->installEventFilter(scrolling);
	this->ui.popSize_GA_spinBox->installEventFilter(scrolling);
	this->ui.eliteRate_GA_doubleSpinBox->installEventFilter(scrolling);
	this->ui.crossoverRate_GA_doubleSpinBox->installEventFilter(scrolling);
	this->ui.mutationRate_GA_doubleSpinBox->installEventFilter(scrolling);
	this->ui.powerFactor_GA_doubleSpinBox->installEventFilter(scrolling);
	this->ui.optCyc_GA_comboBox->installEventFilter(scrolling);
	this->ui.optOffset_GA_comboBox->installEventFilter(scrolling);
	this->ui.optGreen_GA_comboBox->installEventFilter(scrolling);
	this->ui.matchCyc_GA_comboBox->installEventFilter(scrolling);
	//setup Set ID Plan GA combo box
	this->ui.sigPlan_GA_comboBox->addItems(sigSetEntries);
	this->ui.dmdPlan_GA_comboBox->addItems(dmdSetEntries);
	
#ifdef LOCKED_GUI_DEPLOY
	//1800 simtime limit
	this->ui.time_GA_spinBox->setMaximum(1800);
	this->ui.time_Sim_spinBox->setMaximum(1800);
#endif

	// for update all; pass in the struct DefaultParamDataSet pointer to replicate down into each elements
	updateAllValues(dparam);

	// set the default tab widget would always be the first one
	this->ui.DefaultParamInput_tabWidget->setCurrentIndex(0);

	// turn off the cross button of closing the dialog
	this->setWindowFlags(Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
	this->setAttribute(Qt::WA_DeleteOnClose);

	this->setWindowTitle(QString("Default Parameters Settings "));

	this->insertOkButtons();


}

DDefaultParamSettingDialogue::~DDefaultParamSettingDialogue(){

}



// --- --- --- --- --- Settings for Functions --- --- --- --- ---
void DDefaultParamSettingDialogue::setupOKCancelButtons(DDefaultParamSettingFunction* function) {

	//set the text of the buttonsz
	this->ok_Buttons->ref_confirmButton->setText("Submit");;
	this->ok_Buttons->ref_cancelButton->setText("Done");
	this->ok_Buttons->ref_confirmButton->setEnabled(false);

	////connect buttons to function methods
	QObject::connect(this->ok_Buttons->ref_confirmButton, &QPushButton::clicked, function, &DDefaultParamSettingFunction::checkAndSubmitChanges);
	QObject::connect(this->ok_Buttons->ref_confirmButton, &QPushButton::clicked, this, [=]() {setEnableConfirmButton(false); });
	QObject::connect(this->ok_Buttons->ref_cancelButton, &QPushButton::clicked, this, &DDefaultParamSettingDialogue::accept);
	QObject::connect(this, &QDialog::finished, function, &DDefaultParamSettingFunction::dialogFinished);

	// connect flow param changes to enableConfirmButton unlock the submit confirm button
	QObject::connect(ui.NumberOfLane_spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [=]() {setEnableConfirmButton(true); });
	QObject::connect(ui.FreeFlowSpeedLabel_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [=]() {setEnableConfirmButton(true); });
	QObject::connect(ui.BackwardShockwaveSpeed_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [=]() {setEnableConfirmButton(true); });
	QObject::connect(ui.JamDensity_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [=]() {setEnableConfirmButton(true); });
	QObject::connect(ui.SaturationFlow_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [=]() {setEnableConfirmButton(true); });
	QObject::connect(ui.isShowLastInput_comboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [=]() {setEnableConfirmButton(true); });

	// connect Sim param changes to enableConfirmButton unlock the submit confirm button
	QObject::connect(ui.sigPlan_Sim_comboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [=]() {setEnableConfirmButton(true); });
	QObject::connect(ui.dmdPlan_Sim_comboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [=]() {setEnableConfirmButton(true); });
	QObject::connect(ui.time_Sim_spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [=]() {setEnableConfirmButton(true); });

	// connect GA param changes to enableConfirmButton unlock the submit confirm button
	QObject::connect(ui.sigPlan_GA_comboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [=]() {setEnableConfirmButton(true); });
	QObject::connect(ui.dmdPlan_GA_comboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [=]() {setEnableConfirmButton(true); });
	QObject::connect(ui.time_GA_spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [=]() {setEnableConfirmButton(true); });

	QObject::connect(ui.gaType_GA_comboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [=]() {setEnableConfirmButton(true); });
	QObject::connect(ui.threadNum_spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [=]() {setEnableConfirmButton(true); });
	QObject::connect(ui.genNum_GA_spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [=]() {setEnableConfirmButton(true); });
	QObject::connect(ui.subGenNum_GA_spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [=]() {setEnableConfirmButton(true); });
	QObject::connect(ui.popSize_GA_spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [=]() {setEnableConfirmButton(true); });
	QObject::connect(ui.eliteRate_GA_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [=]() {setEnableConfirmButton(true); });
	QObject::connect(ui.crossoverRate_GA_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [=]() {setEnableConfirmButton(true); });
	QObject::connect(ui.mutationRate_GA_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [=]() {setEnableConfirmButton(true); });
	QObject::connect(ui.powerFactor_GA_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [=]() {setEnableConfirmButton(true); });
	QObject::connect(ui.optCyc_GA_comboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [=]() {setEnableConfirmButton(true); });
	QObject::connect(ui.optOffset_GA_comboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [=]() {setEnableConfirmButton(true); });
	QObject::connect(ui.optGreen_GA_comboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [=]() {setEnableConfirmButton(true); });
	QObject::connect(ui.matchCyc_GA_comboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [=]() {setEnableConfirmButton(true); });

}

void DDefaultParamSettingDialogue::setEnableConfirmButton(bool enable) {
	this->ok_Buttons->ref_confirmButton->setEnabled(enable);
}

// --- --- --- --- --- show error tool --- --- --- --- --- 
void DDefaultParamSettingDialogue::failMessage(QString&& msg) {
	this->ok_Buttons->ref_feedbackLabel->setText(msg);
	this->ok_Buttons->ref_feedbackLabel->update();
}

// --- --- --- --- --- Set values --- --- --- --- ---
// Default Setting for Flow Parameters
void DDefaultParamSettingDialogue::setNumberOfLane(int numOfLane){
	this->ui.NumberOfLane_spinBox->setValue(numOfLane);
}

void DDefaultParamSettingDialogue::setFreeFlowSpeed(double v){
	this->ui.FreeFlowSpeedLabel_doubleSpinBox->setValue(v);
}

void DDefaultParamSettingDialogue::setBackwardShockwaveSpeed(double w){
	this->ui.BackwardShockwaveSpeed_doubleSpinBox->setValue(w);
}

void DDefaultParamSettingDialogue::setJamDensity(double k){
	this->ui.JamDensity_doubleSpinBox->setValue(k * 1000);
}

void DDefaultParamSettingDialogue::setSaturationFlow(double q){
	this->ui.SaturationFlow_doubleSpinBox->setValue(q * 3600);
}

void DDefaultParamSettingDialogue::setisShowLastInput(bool enable){
	this->ui.isShowLastInput_comboBox->setCurrentIndex(enable);
}


// Default Setting for Sim
void DDefaultParamSettingDialogue::setSimSigSetID(int setID){
	//All the sig/dmd set encodeing and decodeing are the same why separate them,
	// both shoudl live in the same cpp file -JLo

	// coz the string item that input into the combobox slots r with the setID + the name as well
	for (int index = 0; index < this->ui.sigPlan_Sim_comboBox->count(); index++) {
		bool isAvailable;
		int converted_setID = this->ui.sigPlan_Sim_comboBox->itemText(index).split(QRegExp("\\s+"), QString::SkipEmptyParts)[0].toInt(&isAvailable);
		if (isAvailable && converted_setID == setID) {
			this->ui.sigPlan_Sim_comboBox->setCurrentIndex(index);
		}
		// else not found so dont set that inputted setID into the combobox
	}
}

void DDefaultParamSettingDialogue::setSimDmdSetID(int setID){
	// coz the string item that input into the combobox slots r with the setID + the name as well
	for (int index = 0; index < this->ui.dmdPlan_Sim_comboBox->count(); index++) {
		bool isAvailable;
		int converted_setID = this->ui.dmdPlan_Sim_comboBox->itemText(index).split(QRegExp("\\s+"), QString::SkipEmptyParts)[0].toInt(&isAvailable);
		if (isAvailable && converted_setID == setID) {
			this->ui.dmdPlan_Sim_comboBox->setCurrentIndex(index);
		}
		// else not found so dont set that inputted setID into the combobox
	}
}

void DDefaultParamSettingDialogue::setSimTimeInput(int time){
	this->ui.time_Sim_spinBox->setValue(time);
}


// Default Setting for GA
void DDefaultParamSettingDialogue::setGASigSetID(int setID){
	// coz the string item that input into the combobox slots r with the setID + the name as well
	for (int index = 0; index < this->ui.sigPlan_GA_comboBox->count(); index++) {
		bool isAvailable;
		int converted_setID = this->ui.sigPlan_GA_comboBox->itemText(index).split(QRegExp("\\s+"), QString::SkipEmptyParts)[0].toInt(&isAvailable);
		if (isAvailable && converted_setID == setID) {
			this->ui.sigPlan_GA_comboBox->setCurrentIndex(index);
		}
		// else not found so dont set that inputted setID into the combobox
	}
}

void DDefaultParamSettingDialogue::setGADmdSetID(int setID){
	// coz the string item that input into the combobox slots r with the setID + the name as well
	for (int index = 0; index < this->ui.dmdPlan_GA_comboBox->count(); index++) {
		bool isAvailable;
		int converted_setID = this->ui.dmdPlan_GA_comboBox->itemText(index).split(QRegExp("\\s+"), QString::SkipEmptyParts)[0].toInt(&isAvailable);
		if (isAvailable && converted_setID == setID) {
			this->ui.dmdPlan_GA_comboBox->setCurrentIndex(index);
		}
		// else not found so dont set that inputted setID into the combobox
	}
}

void DDefaultParamSettingDialogue::setGATimeInput(int time){
	this->ui.time_GA_spinBox->setValue(time);
}


void DDefaultParamSettingDialogue::setGAType(int Index){
	this->ui.gaType_GA_comboBox->setCurrentIndex(Index);
}

void DDefaultParamSettingDialogue::setNumOfThreads(int numOfThread) {
	this->ui.threadNum_spinBox->setValue(numOfThread);
}

void DDefaultParamSettingDialogue::setGANumOfGen(int numOfgen){
	this->ui.genNum_GA_spinBox->setValue(numOfgen);
}

void DDefaultParamSettingDialogue::setGANumOfSubGen(int numOfsubgen){
	this->ui.subGenNum_GA_spinBox->setValue(numOfsubgen);
}

void DDefaultParamSettingDialogue::setGAPopSize(int numOfpop){
	this->ui.popSize_GA_spinBox->setValue(numOfpop);
}

void DDefaultParamSettingDialogue::setGAEliteRate(double rate){
	this->ui.eliteRate_GA_doubleSpinBox->setValue(rate);
}

void DDefaultParamSettingDialogue::setGACrossoverRate(double rate){
	this->ui.crossoverRate_GA_doubleSpinBox->setValue(rate);
}

void DDefaultParamSettingDialogue::setGAMutationRate(double rate){
	this->ui.mutationRate_GA_doubleSpinBox->setValue(rate);
}

void DDefaultParamSettingDialogue::setGAPowerFactor(double rate){
	this->ui.powerFactor_GA_doubleSpinBox->setValue(rate);
}

void DDefaultParamSettingDialogue::setGAOptCyc(bool enable){
	this->ui.optCyc_GA_comboBox->setCurrentIndex(enable);
}

void DDefaultParamSettingDialogue::setGAOptOffset(bool enable){
	this->ui.optOffset_GA_comboBox->setCurrentIndex(enable);
}

void DDefaultParamSettingDialogue::setGAOptGreen(bool enable){
	this->ui.optGreen_GA_comboBox->setCurrentIndex(enable);
}

void DDefaultParamSettingDialogue::setGAMatchCyc(bool enable){
	this->ui.matchCyc_GA_comboBox->setCurrentIndex(enable);
}

// --- --- --- --- --- update all values --- --- --- --- ---
// pass in the struct DefaultParamDataSet pointer to replicate down into each elements
void DDefaultParamSettingDialogue::updateAllValues(DefaultParamDataSet* dparam) {
	//why go though all the trouble of creating all the set function
	// but are all only used internally? -JLo

	// record the changes on Default Flow Parameters
	setNumberOfLane(dparam->m_FlowParams.NumberOfLane);
	setFreeFlowSpeed(dparam->m_FlowParams.FreeFlowSpeed);
	setBackwardShockwaveSpeed(dparam->m_FlowParams.BackwardShockwaveSpeed);
	setJamDensity(dparam->m_FlowParams.JamDensity);
	setSaturationFlow(dparam->m_FlowParams.SaturationFlow);
	setisShowLastInput(dparam->isShowLastInput);

	// record the changes on Default Sim Parameters
	setSimSigSetID(dparam->m_runSimParams.sigsetID);
	setSimDmdSetID(dparam->m_runSimParams.dmdsetID);
	setSimTimeInput(dparam->m_runSimParams.runTime);

	// record the changes on Default GA Parameters
	setGASigSetID(dparam->m_runGAParams.sigsetID);
	setGADmdSetID(dparam->m_runGAParams.dmdsetID);
	setGATimeInput(dparam->m_runGAParams.runTime);

	setGAType(dparam->m_runGAParams.gaType);
	setNumOfThreads(dparam->m_runGAParams.threadNum);
	setGANumOfGen(dparam->m_runGAParams.genNum);
	setGANumOfSubGen(dparam->m_runGAParams.subGenNum);
	setGAPopSize(dparam->m_runGAParams.popSize);
	setGAEliteRate(dparam->m_runGAParams.eliteRate);
	setGACrossoverRate(dparam->m_runGAParams.crossoverRate);
	setGAMutationRate(dparam->m_runGAParams.mutationRate);
	setGAPowerFactor(dparam->m_runGAParams.powerFactor);
	setGAOptCyc(dparam->m_runGAParams.isoptCyc);
	setGAOptOffset(dparam->m_runGAParams.isoptOffset);
	setGAOptGreen(dparam->m_runGAParams.isoptGreen);
	setGAMatchCyc(dparam->m_runGAParams.ismatchCyc);

}


// --- --- --- --- --- getter --- --- --- --- ---
// get Default Flow Parameters
int DDefaultParamSettingDialogue::getNumberOfLane() {
	return this->ui.NumberOfLane_spinBox->value();
}

double DDefaultParamSettingDialogue::getFreeFlowSpeed() {
	return this->ui.FreeFlowSpeedLabel_doubleSpinBox->value();
}

double DDefaultParamSettingDialogue::getBackwardShockwaveSpeed() {
	return this->ui.BackwardShockwaveSpeed_doubleSpinBox->value();
}

double DDefaultParamSettingDialogue::getJamDensity() {
	return this->ui.JamDensity_doubleSpinBox->value()/1000;
}

double DDefaultParamSettingDialogue::getSaturationFlow() {
	return this->ui.SaturationFlow_doubleSpinBox->value()/3600;
}

bool DDefaultParamSettingDialogue::getisShowLastInput() {
	return this->ui.isShowLastInput_comboBox->currentIndex();
}


// get Default Sim Parameters
QString DDefaultParamSettingDialogue::getSimSigSetInput() {
	return this->ui.sigPlan_Sim_comboBox->currentText();
}

int DDefaultParamSettingDialogue::getSimSigSetID() {
	if (this->ui.sigPlan_Sim_comboBox->count()) {
		bool isAvailable = false;
		int converted_setID = this->ui.sigPlan_Sim_comboBox->currentText().split(QRegExp("\\s+"), QString::SkipEmptyParts)[0].toInt(&isAvailable);
		if (isAvailable)
			return converted_setID;
	}
	return -1;
}

QString DDefaultParamSettingDialogue::getSimDmdSetInput() {
	return this->ui.dmdPlan_Sim_comboBox->currentText();
}

int DDefaultParamSettingDialogue::getSimDmdSetID() {
	if (this->ui.dmdPlan_Sim_comboBox->count()) {
		bool isAvailable = false;
		int converted_setID = this->ui.dmdPlan_Sim_comboBox->currentText().split(QRegExp("\\s+"), QString::SkipEmptyParts)[0].toInt(&isAvailable);
		if (isAvailable)
			return converted_setID;
	}
	return -1;
}

int DDefaultParamSettingDialogue::getSimTimeInput() {
	return this->ui.time_Sim_spinBox->value();
}


// get Default GA Parameters
QString DDefaultParamSettingDialogue::getGASigSetInput() {
	return this->ui.sigPlan_GA_comboBox->currentText();
}

int DDefaultParamSettingDialogue::getGASigSetID() {
	if (this->ui.sigPlan_GA_comboBox->count()) {
		bool isAvailable = false;
		int converted_setID = this->ui.sigPlan_GA_comboBox->currentText().split(QRegExp("\\s+"), QString::SkipEmptyParts)[0].toInt(&isAvailable);
		if (isAvailable)
			return converted_setID;
	}
	return -1;
}

QString DDefaultParamSettingDialogue::getGADmdSetInput() {
	return this->ui.dmdPlan_GA_comboBox->currentText();
}

int DDefaultParamSettingDialogue::getGADmdSetID() {
	if (this->ui.dmdPlan_GA_comboBox->count()) {
		bool isAvailable = false;
		int converted_setID = this->ui.dmdPlan_GA_comboBox->currentText().split(QRegExp("\\s+"), QString::SkipEmptyParts)[0].toInt(&isAvailable);
		if (isAvailable)
			return converted_setID;
	}
	return -1;
}

int DDefaultParamSettingDialogue::getGATimeInput() {
	return this->ui.time_GA_spinBox->value();
}


int DDefaultParamSettingDialogue::getGAType() {
	return this->ui.gaType_GA_comboBox->currentIndex();
}

int DDefaultParamSettingDialogue::getNumOfThreads() {
	return this->ui.threadNum_spinBox->value();
}

int DDefaultParamSettingDialogue::getGANumOfGen() {
	return this->ui.genNum_GA_spinBox->value();
}

int DDefaultParamSettingDialogue::getGANumOfSubGen() {
	return this->ui.subGenNum_GA_spinBox->value();
}

int DDefaultParamSettingDialogue::getGAPopSize() {
	return this->ui.popSize_GA_spinBox->value();
}

double DDefaultParamSettingDialogue::getGAEliteRate() {
	return this->ui.eliteRate_GA_doubleSpinBox->value();
}

double DDefaultParamSettingDialogue::getGACrossoverRate() {
	return this->ui.crossoverRate_GA_doubleSpinBox->value();
}

double DDefaultParamSettingDialogue::getGAMutationRate() {
	return this->ui.mutationRate_GA_doubleSpinBox->value();
}

double DDefaultParamSettingDialogue::getGAPowerFactor() {
	return this->ui.powerFactor_GA_doubleSpinBox->value();
}

bool DDefaultParamSettingDialogue::getGAOptCyc() {
	return this->ui.optCyc_GA_comboBox->currentIndex();
}

bool DDefaultParamSettingDialogue::getGAOptOffset() {
	return this->ui.optOffset_GA_comboBox->currentIndex();
}

bool DDefaultParamSettingDialogue::getGAOptGreen() {
	return this->ui.optGreen_GA_comboBox->currentIndex();
}

bool DDefaultParamSettingDialogue::getGAMatchCyc() {
	return this->ui.matchCyc_GA_comboBox->currentIndex();
}



