#include "DSignalTimingPlanDialogue.h"


//DISCO GUI stuff
#include "../Functions/Junction/DSignalTimingPlanCreateFunc.h"
#include "DOkCancelButtons.h"
#include "GraphicsItems/DCellConnectorItem.h"
#include "Utils/MouseWheelWidgetAdjustmentGuard.h"

//Qt lib
#include <QString>
#include <QtWidgets/QLabel>

//DISCO Core lib
#include "Editor_Sig.h"
#include "Sig_Container.h"
#include "Net_Container.h"

// --- --- --- --- --- Private Utils --- --- --- --- ---
// insert external button, text
void DSignalTimingPlanDialogue::insertOkButtons() {
	this->ok_Buttons = new DOkCancelButtons(this);
	this->ok_Buttons->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	//always put the okbutton at bottom after the spacer
	int numOfWidget = this->ui.verticalLayoutOfDialogue->count();
	this->ui.verticalLayoutOfDialogue->insertWidget(numOfWidget, this->ok_Buttons);
}


void DSignalTimingPlanDialogue::updateMaxLimitToGreenOnOffTime() {
	// get the selected row
	int total_TableRowCount = ui.TimingIput_tableWidget->rowCount();
	int CycleTime = ui.CycleTime_spinBox->value();
	for (int cur_row = 0; cur_row < total_TableRowCount; cur_row++) {

		//TODO mod the index
		// getting each QTableWidgetItem items row by row
		QSpinBox* GOnTCell = static_cast<QSpinBox*>(ui.TimingIput_tableWidget->cellWidget(cur_row, 1));
		QSpinBox* GOffTCell = static_cast<QSpinBox*>(ui.TimingIput_tableWidget->cellWidget(cur_row, 2));
		QSpinBox* G2OnTCell = static_cast<QSpinBox*>(ui.TimingIput_tableWidget->cellWidget(cur_row, 4));
		QSpinBox* G2OffTCell = static_cast<QSpinBox*>(ui.TimingIput_tableWidget->cellWidget(cur_row, 5));

		// reset the max limit of each green input time cell
		GOnTCell->setMaximum(CycleTime);
		GOffTCell->setMaximum(CycleTime);
		G2OnTCell->setMaximum(CycleTime);
		G2OffTCell->setMaximum(CycleTime);
	}
}



// --- --- --- --- --- Constructor Destructor --- --- --- --- ---
DSignalTimingPlanDialogue::DSignalTimingPlanDialogue() : QDialog(nullptr) {
	ui.setupUi(this);

	// NO MORE SCROLLING
	this->noMoreScrolling = new MouseWheelWidgetAdjustmentGuard{ this };
	ui.CycleTime_spinBox->installEventFilter(this->noMoreScrolling);
	ui.Offset_spinBox->installEventFilter(this->noMoreScrolling);

	// turn off the cross button of closing the dialog
	this->setWindowFlags(Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
	this->setAttribute(Qt::WA_DeleteOnClose);

	this->setWindowTitle(QString("Edit Signal Timing Plan"));

	this->insertOkButtons();



}

DSignalTimingPlanDialogue::~DSignalTimingPlanDialogue(){


}


// --- --- --- --- --- Settings for Functions --- --- --- --- ---
void DSignalTimingPlanDialogue::setupCreateButtons(DSignalTimingPlanCreateFunc* function) {
	//set the text of the buttonsz
	this->ok_Buttons->ref_confirmButton->setText("Submit");;
	this->ok_Buttons->ref_cancelButton->setText("Close");
	this->ok_Buttons->ref_confirmButton->setEnabled(false);

	//connect buttons to function methods
	QObject::connect(this->ok_Buttons->ref_confirmButton, &QPushButton::clicked, function, &DSignalTimingPlanCreateFunc::checkAndSubmitChanges);
	QObject::connect(this->ok_Buttons->ref_cancelButton, &QPushButton::clicked, this, &DSignalTimingPlanDialogue::accept);
	QObject::connect(this, &QDialog::finished, function, &DSignalTimingPlanCreateFunc::dialogFinished);

	////connect internal changes to confirm button
	QObject::connect(ui.SigsetcomboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DSignalTimingPlanDialogue::enableConfirmButton);
	
	// basically just connect the cycle time and offset time spinbox changes to enableConfirmButton unlock the submit confirm button
	QObject::connect(ui.CycleTime_spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &DSignalTimingPlanDialogue::enableConfirmButton);
	QObject::connect(ui.Offset_spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &DSignalTimingPlanDialogue::enableConfirmButton);
}

void DSignalTimingPlanDialogue::setref_confirmButtonEnabled(bool IsEnable) {
	this->ok_Buttons->ref_confirmButton->setEnabled(IsEnable);
}

void DSignalTimingPlanDialogue::enableConfirmButton() {
	this->ok_Buttons->ref_confirmButton->setEnabled(true);
}

void DSignalTimingPlanDialogue::connectSigSetComboBox(DSignalTimingPlanCreateFunc* function) {
	// connect the type of sigset combobox switch
	QObject::connect(ui.SigsetcomboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), function, &DSignalTimingPlanCreateFunc::UpdateChartAndTableBySigSetSwitch);
}

void DSignalTimingPlanDialogue::connectDataChanged(DSignalTimingPlanCreateFunc* function) {
	// change of cycle time would limit the max spin box val input for green time
	QObject::connect(ui.CycleTime_spinBox, &QSpinBox::editingFinished, this, &DSignalTimingPlanDialogue::updateMaxLimitToGreenOnOffTime);
	// change of any value in green time input will refreshing of chart view by updating ChartContent
	QObject::connect(ui.CycleTime_spinBox, &QSpinBox::editingFinished, function, &DSignalTimingPlanCreateFunc::BuildUpChartContentFromTable);

	// basically just connect the cycle time and offset time spinbox changes to function->DataChanged
	QObject::connect(ui.CycleTime_spinBox, QOverload<int>::of(&QSpinBox::valueChanged), function, &DSignalTimingPlanCreateFunc::DataChanged);
	QObject::connect(ui.Offset_spinBox, QOverload<int>::of(&QSpinBox::valueChanged), function, &DSignalTimingPlanCreateFunc::DataChanged);

	QObject::connect(this->ui.TimingIput_tableWidget->selectionModel(), &QItemSelectionModel::selectionChanged,
		function, &DSignalTimingPlanCreateFunc::sigGpSelectionChanged);
}

void DSignalTimingPlanDialogue::setCycleTime(int CycleTime) {
	ui.CycleTime_spinBox->setValue(CycleTime);
}

void DSignalTimingPlanDialogue::setOffset(int OffsetTime) {
	ui.Offset_spinBox->setValue(OffsetTime);
}

void DSignalTimingPlanDialogue::setSigSetComboBoxIndex(int index) {
	ui.SigsetcomboBox->setCurrentIndex(index);
}


//  --- --- --- --- --- provide direct interaction with the TimingIput_tableWidget  --- --- --- --- ---
void DSignalTimingPlanDialogue::ClearAllContentInTable() { 
	int total_TableRowCount = ui.TimingIput_tableWidget->rowCount();

	for (int i = total_TableRowCount - 1; i >= 0; i--) {
		ui.TimingIput_tableWidget->removeRow(i);
	}

}

void DSignalTimingPlanDialogue::addRowToTable(DSignalTimingPlanCreateFunc* function, int SigGpID, int GreenOnTime, int GreenOffTime, int Green2OnTime, int Green2OffTime) {
	// get total row count
	int row = ui.TimingIput_tableWidget->rowCount();
	ui.TimingIput_tableWidget->insertRow(row);

	// SigGpID Cell set up 
	QSpinBox* SigGpIDCell = new QSpinBox(ui.TimingIput_tableWidget);
	SigGpIDCell->setMinimum(0);
	SigGpIDCell->setMaximum(INT_MAX);
	SigGpIDCell->setValue(SigGpID);
	SigGpIDCell->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	SigGpIDCell->setButtonSymbols(QAbstractSpinBox::NoButtons);
	SigGpIDCell->setReadOnly(true);
	SigGpIDCell->setFrame(false);  // no frame
	ui.TimingIput_tableWidget->setCellWidget(row, 0, SigGpIDCell);

	// GreenOnTime Cell set up 
	QSpinBox* GOnTCell = new QSpinBox(ui.TimingIput_tableWidget);
	GOnTCell->setMinimum(-1);
	GOnTCell->setSpecialValueText(" ");
	GOnTCell->setMaximum(ui.CycleTime_spinBox->value());
	GOnTCell->setValue(GreenOnTime);
	GOnTCell->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	GOnTCell->setButtonSymbols(QAbstractSpinBox::PlusMinus);
	GOnTCell->setReadOnly(false);
	GOnTCell->setFrame(false);  // no frame
	GOnTCell->installEventFilter(this->noMoreScrolling);
	ui.TimingIput_tableWidget->setCellWidget(row, 1, GOnTCell);
	

	// GreenOffTime Cell set up 
	QSpinBox* GOffTCell = new QSpinBox(ui.TimingIput_tableWidget);
	GOffTCell->setMinimum(-1);
	GOffTCell->setSpecialValueText(" ");
	GOffTCell->setMaximum(ui.CycleTime_spinBox->value());
	GOffTCell->setValue(GreenOffTime);
	GOffTCell->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	GOffTCell->setButtonSymbols(QAbstractSpinBox::PlusMinus);
	GOffTCell->setReadOnly(false);
	GOffTCell->setFrame(false);  // no frame
	GOffTCell->installEventFilter(this->noMoreScrolling);
	ui.TimingIput_tableWidget->setCellWidget(row, 2, GOffTCell);

	//reset button 1
	QPushButton* G1Reset = new QPushButton(ui.TimingIput_tableWidget);
	G1Reset->setText("Reset");
	G1Reset->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	ui.TimingIput_tableWidget->setCellWidget(row, 3, G1Reset);

	// Green2OnTime Cell set up 
	QSpinBox* G2OnTCell = new QSpinBox(ui.TimingIput_tableWidget);
	G2OnTCell->setMinimum(-1);
	G2OnTCell->setSpecialValueText(" ");
	G2OnTCell->setMaximum(ui.CycleTime_spinBox->value());
	G2OnTCell->setValue(Green2OnTime);
	G2OnTCell->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	G2OnTCell->setButtonSymbols(QAbstractSpinBox::PlusMinus);
	G2OnTCell->setReadOnly(false);
	G2OnTCell->setFrame(false);  // no frame
	G2OnTCell->installEventFilter(this->noMoreScrolling);
	ui.TimingIput_tableWidget->setCellWidget(row, 4, G2OnTCell);

	// Green2OffTime Cell set up 
	QSpinBox* G2OffTCell = new QSpinBox(ui.TimingIput_tableWidget);
	G2OffTCell->setMinimum(-1);
	G2OffTCell->setSpecialValueText(" ");
	G2OffTCell->setMaximum(ui.CycleTime_spinBox->value());
	G2OffTCell->setValue(Green2OffTime);
	G2OffTCell->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	G2OffTCell->setButtonSymbols(QAbstractSpinBox::PlusMinus);
	G2OffTCell->setReadOnly(false);
	G2OffTCell->setFrame(false);  // no frame
	G2OffTCell->installEventFilter(this->noMoreScrolling);
	ui.TimingIput_tableWidget->setCellWidget(row, 5, G2OffTCell);

	//reset button 2
	QPushButton* G2Reset = new QPushButton(ui.TimingIput_tableWidget);
	G2Reset->setText("Reset");
	G2Reset->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	ui.TimingIput_tableWidget->setCellWidget(row, 6, G2Reset);


	QObject::connect(GOnTCell, QOverload<int>::of(&QSpinBox::valueChanged), this, &DSignalTimingPlanDialogue::enableConfirmButton);
	QObject::connect(GOffTCell, QOverload<int>::of(&QSpinBox::valueChanged), this, &DSignalTimingPlanDialogue::enableConfirmButton);
	QObject::connect(G2OnTCell, QOverload<int>::of(&QSpinBox::valueChanged), this, &DSignalTimingPlanDialogue::enableConfirmButton);
	QObject::connect(G2OffTCell, QOverload<int>::of(&QSpinBox::valueChanged), this, &DSignalTimingPlanDialogue::enableConfirmButton);

	// change of any value in green time input will recorded down which used to required the save submition of data changes on corresponding sigset
	QObject::connect(GOnTCell, QOverload<int>::of(&QSpinBox::valueChanged), function, &DSignalTimingPlanCreateFunc::DataChanged);
	QObject::connect(GOffTCell, QOverload<int>::of(&QSpinBox::valueChanged), function, &DSignalTimingPlanCreateFunc::DataChanged);
	QObject::connect(G2OnTCell, QOverload<int>::of(&QSpinBox::valueChanged), function, &DSignalTimingPlanCreateFunc::DataChanged);
	QObject::connect(G2OffTCell, QOverload<int>::of(&QSpinBox::valueChanged), function, &DSignalTimingPlanCreateFunc::DataChanged);

	// change of any value in green time input will refreshing of chart view by updating ChartContent
	QObject::connect(GOnTCell, QOverload<int>::of(&QSpinBox::valueChanged), function, &DSignalTimingPlanCreateFunc::BuildUpChartContentFromTable);
	QObject::connect(GOffTCell, QOverload<int>::of(&QSpinBox::valueChanged), function, &DSignalTimingPlanCreateFunc::BuildUpChartContentFromTable);
	QObject::connect(G2OnTCell, QOverload<int>::of(&QSpinBox::valueChanged), function, &DSignalTimingPlanCreateFunc::BuildUpChartContentFromTable);
	QObject::connect(G2OffTCell, QOverload<int>::of(&QSpinBox::valueChanged), function, &DSignalTimingPlanCreateFunc::BuildUpChartContentFromTable);
	// Noted : if use signal editingFinished instead here; would cause error when calling ClearAllContentInTable, coz the tablewidgetitem might be still 
	// being used havent been checked out by clicking any other widget. therfore that spinbox ptr cant be removed and destroyed

	//Reset buttons
	QObject::connect(G1Reset, &QPushButton::clicked, GOnTCell, [=]() {GOnTCell->setValue(-1); });
	QObject::connect(G1Reset, &QPushButton::clicked, GOffTCell, [=]() {GOffTCell->setValue(-1); });
	QObject::connect(G2Reset, &QPushButton::clicked, G2OnTCell, [=]() {G2OnTCell->setValue(-1); });
	QObject::connect(G2Reset, &QPushButton::clicked, G2OffTCell, [=]() {G2OffTCell->setValue(-1); });
}


void DSignalTimingPlanDialogue::OverwriteRowToTableRegardsToSigGpID(int SigGpID, int GreenOnTime, int GreenOffTime, int Green2OnTime, int Green2OffTime) {
	// get total row count
	int total_TableRowCount = ui.TimingIput_tableWidget->rowCount();
	int row;
	for (int cur_row = 0; cur_row < total_TableRowCount; cur_row++) {
		// getting each QTableWidgetItem items row by row
		QSpinBox* SigGpIDCell = static_cast<QSpinBox*>(ui.TimingIput_tableWidget->cellWidget(cur_row, 0));

		// check if the SigGpID have already exist; yes then break loop and overwrite green val
		if (SigGpIDCell->value() == SigGpID) {
			row = cur_row;
			break;
		}
	}

	try {
		// GreenOnTime Cell set up 
		QSpinBox* GOnTCell = static_cast<QSpinBox*>(ui.TimingIput_tableWidget->cellWidget(row, 1));
		GOnTCell->setValue(GreenOnTime);

		// GreenOffTime Cell set up 
		QSpinBox* GOffTCell = static_cast<QSpinBox*>(ui.TimingIput_tableWidget->cellWidget(row, 2));
		GOffTCell->setValue(GreenOffTime);

		//TODO mod the index
		// GreenOnTime Cell set up 
		QSpinBox* G2OnTCell = static_cast<QSpinBox*>(ui.TimingIput_tableWidget->cellWidget(row, 4));
		G2OnTCell->setValue(Green2OnTime);

		// GreenOffTime Cell set up 
		QSpinBox* G2OffTCell = static_cast<QSpinBox*>(ui.TimingIput_tableWidget->cellWidget(row, 5));
		G2OffTCell->setValue(Green2OffTime);
	}
	catch (...) {
		this->failMessage({ "SigGpID is not exist" });
	}
}




// --- --- --- --- --- show error tool --- --- --- --- --- 
void DSignalTimingPlanDialogue::failMessage(QString&& msg) {
	this->ok_Buttons->ref_feedbackLabel->setText(msg);
	this->ok_Buttons->ref_feedbackLabel->update();
}

// --- --- --- --- --- getter --- --- --- --- ---
int DSignalTimingPlanDialogue::getCycleTime() {
	return ui.CycleTime_spinBox->value();
}

int DSignalTimingPlanDialogue::getOffset() {
	return ui.Offset_spinBox->value();
}

int DSignalTimingPlanDialogue::getSigSetComboBoxIndex() {
	// QSrtring of the sigset
	return this->ui.SigsetcomboBox->currentIndex();
}


Ui::DSignalTimingPlanDialogue DSignalTimingPlanDialogue::getUI() {
	return this->ui;
}
