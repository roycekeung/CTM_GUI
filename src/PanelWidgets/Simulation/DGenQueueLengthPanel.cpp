#include "DGenQueueLengthPanel.h"

#include "Functions/Simulation/DGenQueueLengthFunction.h"
#include "PanelWidgets/DOkCancelButtons.h"

const std::string DGenQueueLengthPanel::hintText{
"Select a cell where queue will be counted from\r\n\
Left click on a cell to select\r\n\
\r\n\
Green cell is start cell(inclusive)\r\n\
Red cells are queue length cut-offs(exclusive)\r\n\
\r\n\
A threshold rate is required to determine if it is queuing\r\n\
the value should be the loaded fraction (0-1)\r\n\
\r\n"
};

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DGenQueueLengthPanel::DGenQueueLengthPanel() : QWidget(nullptr) {
	ui.setupUi(this);

	this->ui.hint_label->setText(QString::fromStdString(this->hintText));
	this->ui.startCell_spinBox->setSpecialValueText(" ");

	//ok cancel buttons
	this->m_okButtons = new DOkCancelButtons(this);
	this->m_okButtons->ref_confirmButton->setText("Generate Queue Lengths");
	this->ui.verticalLayout_3->insertWidget(4, this->m_okButtons);
}

DGenQueueLengthPanel::~DGenQueueLengthPanel() {}

void DGenQueueLengthPanel::connectFunction(DGenQueueLengthFunction* function) {
	QObject::connect(this->ui.startCellReset_pushButton, &QPushButton::clicked, function, &DGenQueueLengthFunction::reselectStartCell);
	QObject::connect(this->ui.remove_pushButton, &QPushButton::clicked, this, &DGenQueueLengthPanel::clickedRemove);
	QObject::connect(this, &DGenQueueLengthPanel::removeEndCell, function, &DGenQueueLengthFunction::removeEndCell);
	QObject::connect(this->ui.removeAll_pushButton, &QPushButton::clicked, function, &DGenQueueLengthFunction::removeAllEndCell);
	QObject::connect(this->m_okButtons->ref_confirmButton, &QPushButton::clicked, function, &DGenQueueLengthFunction::confirmCellList);
	QObject::connect(this->m_okButtons->ref_cancelButton, &QPushButton::clicked, function, &DGenQueueLengthFunction::cancel);
}

// --- --- --- --- --- Settings for Functions --- --- --- --- ---

void DGenQueueLengthPanel::setRecStartEndTime(int startTime, int endTime) {
	this->ui.startTime_spinBox->setMinimum(startTime);
	this->ui.startTime_spinBox->setMaximum(endTime);
	this->ui.startTime_spinBox->setValue(startTime);
	this->ui.endTime_spinBox->setMinimum(startTime);
	this->ui.endTime_spinBox->setMaximum(endTime);
	this->ui.endTime_spinBox->setValue(endTime);
}

void DGenQueueLengthPanel::setStartCellId(int cellId) {
	this->ui.startCell_spinBox->setValue(cellId);
}

void DGenQueueLengthPanel::resetList(const std::unordered_set<int>& cellIds) {
	this->ui.endCells_tableWidget->setRowCount(0);
	this->ui.endCells_tableWidget->setRowCount(cellIds.size());

	int count = 0;
	for (const int& cellId : cellIds) {
		QLabel* idCell = new QLabel(this->ui.endCells_tableWidget);
		idCell->setText(std::to_string(cellId).c_str());
		idCell->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
		this->ui.endCells_tableWidget->setCellWidget(count, 0, idCell);
		count++;
	}
}

// --- --- --- --- --- Getters --- --- --- --- ---

int DGenQueueLengthPanel::getStartTime() {
	return this->ui.startTime_spinBox->value();
}

int DGenQueueLengthPanel::getEndTime() {
	return this->ui.endTime_spinBox->value();
}

double DGenQueueLengthPanel::getThresholdRate() {
	return this->ui.thresholdRate_doubleSpinBox->value();
}

// --- --- --- --- --- Redirect Slots --- --- --- --- ---

void DGenQueueLengthPanel::clickedRemove() {
	auto selected = this->ui.endCells_tableWidget->selectionModel()->selectedRows();
	if (selected.size() == 1) {
		QLabel* idCell = dynamic_cast<QLabel*>(this->ui.endCells_tableWidget->cellWidget(selected.first().row(), 0));
		if (idCell)
			emit this->removeEndCell(idCell->text().toInt());
	}
}