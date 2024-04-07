#include "DGenDelayPlotPanel.h"

#include "Functions/Simulation/DGenDelayPlotFunction.h"

#include "I_RecordsHandler.h"

const std::string DGenDelayPlotPanel::hintText{
"Select the cells and the order to be printed out\r\n\
Left click on a cell to select\r\n\
Program will attempt to connect a path of cells\r\n\
Shift left click to only select the single cell\r\n"
};

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DGenDelayPlotPanel::DGenDelayPlotPanel() : QWidget(nullptr) {
	ui.setupUi(this);
	this->ui.hint_label->setText(QString::fromStdString(this->hintText));

#ifndef LOCKED_GUI_DEPLOY
	this->ui.plotType_comboBox->addItem("y_in");
	this->ui.plotType_comboBox->addItem("y_new");
	this->ui.plotType_comboBox->addItem("y_out");
	this->ui.plotType_comboBox->addItem("all");
#endif
}

DGenDelayPlotPanel::~DGenDelayPlotPanel() {}

void DGenDelayPlotPanel::connectFunction(DGenDelayPlotFunction* function) {
	QObject::connect(this->ui.close_pushButton, &QPushButton::clicked, function, &DGenDelayPlotFunction::cancel);
	QObject::connect(this->ui.genPlot_pushButton, &QPushButton::clicked, function, &DGenDelayPlotFunction::confirmCellList);
	QObject::connect(this->ui.remove_pushButton, &QPushButton::clicked, function, &DGenDelayPlotFunction::removeLast);
	QObject::connect(this->ui.removeAll_pushButton, &QPushButton::clicked, function, &DGenDelayPlotFunction::removeAll);
}

// --- --- --- --- --- Settings for Functions --- --- --- --- ---

void DGenDelayPlotPanel::addRow(int cellId) {
	int row = this->ui.sigGp_tableWidget->rowCount();
	this->ui.sigGp_tableWidget->insertRow(row);

	// pos id set up
	QLabel* idCell = new QLabel(this->ui.sigGp_tableWidget);
	idCell->setText(std::to_string(cellId).c_str());
	idCell->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	this->ui.sigGp_tableWidget->setCellWidget(row, 0, idCell);
}

void DGenDelayPlotPanel::setRecStartEndTime(int startTime, int endTime) {
	this->ui.startTime_spinBox->setMinimum(startTime);
	this->ui.startTime_spinBox->setMaximum(endTime);
	this->ui.startTime_spinBox->setValue(startTime);
	this->ui.endTime_spinBox->setMinimum(startTime);
	this->ui.endTime_spinBox->setMaximum(endTime);
	this->ui.endTime_spinBox->setValue(endTime);
}

int DGenDelayPlotPanel::getPlotType() {
	switch (this->ui.plotType_comboBox->currentIndex()) {
	case 0:
		return (int) DISCO2_API::I_RecordsHandler::recType::Occ;
	case 1:
		return (int) DISCO2_API::I_RecordsHandler::recType::delay;
	case 2:
		return (int)DISCO2_API::I_RecordsHandler::recType::y_in;
	case 3:
		return (int)DISCO2_API::I_RecordsHandler::recType::y_new;
	case 4:
		return (int)DISCO2_API::I_RecordsHandler::recType::y_out;
	case 5:
		return 5;
	default:
		return -1;
	}
}

int DGenDelayPlotPanel::getStartTime() {
	return this->ui.startTime_spinBox->value();
}

int DGenDelayPlotPanel::getEndTime() {
	return this->ui.endTime_spinBox->value();
}

void DGenDelayPlotPanel::removeLastRow() {
	this->ui.sigGp_tableWidget->setRowCount(std::max(0, this->ui.sigGp_tableWidget->rowCount() - 1));
}

void DGenDelayPlotPanel::removeAllRows() {
	this->ui.sigGp_tableWidget->setRowCount(0);
}
