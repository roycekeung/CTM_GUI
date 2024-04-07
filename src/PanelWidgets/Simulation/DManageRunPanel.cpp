#include "DManageRunPanel.h"

#include <QLabel>
#include <QDoubleSpinBox>

#include "Functions/Simulation/DManageRunFunction.h"

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DManageRunPanel::DManageRunPanel() : QDialog() {
	ui.setupUi(this);

	this->setWindowFlags(Qt::Window | Qt::WindowTitleHint);
	this->setAttribute(Qt::WA_DeleteOnClose);
	this->ui.Close_pushButton->setDefault(true);

	QObject::connect(this->ui.Close_pushButton, &QPushButton::clicked, this, &QDialog::close);
}

DManageRunPanel::~DManageRunPanel() {
}

// --- --- --- --- --- Getters --- --- --- --- ---

int DManageRunPanel::getCurrentIndex() {
	if (this->ui.TableWidget->selectionModel()->selectedRows().size())
		return this->ui.TableWidget->selectionModel()->selectedRows().first().row();
	else
		return -1;
}

// --- --- --- --- --- Settings for Functions --- --- --- --- ---

void DManageRunPanel::setupButtons(DManageRunFunction* function) {
	QObject::connect(this->ui.Delete_Button, &QPushButton::clicked, function, &DManageRunFunction::deleteRun);
	QObject::connect(this->ui.PlayBack_Button, &QPushButton::clicked, function, &DManageRunFunction::openPlayback);
}

void DManageRunPanel::clearTable() {
	this->ui.TableWidget->setRowCount(0);
}

void DManageRunPanel::addRow(QString&& name, QString&& details, QString&& sigSet, QString&& dmdSet, 
		QString&& time, double tolDmd, double tolDly, double avgDly) {
	int row = this->ui.TableWidget->rowCount();
	this->ui.TableWidget->insertRow(row);

	QLabel* nameCell = new QLabel(this->ui.TableWidget);
	nameCell->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	nameCell->setText(name);
	this->ui.TableWidget->setCellWidget(row, 0, nameCell);

	QLabel* detailsCell = new QLabel(this->ui.TableWidget);
	detailsCell->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	detailsCell->setText(details);
	this->ui.TableWidget->setCellWidget(row, 1, detailsCell);

	QLabel* sigSetCell = new QLabel(this->ui.TableWidget);
	sigSetCell->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	sigSetCell->setText(sigSet);
	this->ui.TableWidget->setCellWidget(row, 2, sigSetCell);

	QLabel* dmdSetCell = new QLabel(this->ui.TableWidget);
	dmdSetCell->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	dmdSetCell->setText(dmdSet);
	this->ui.TableWidget->setCellWidget(row, 3, dmdSetCell);

	QLabel* timeCell = new QLabel(this->ui.TableWidget);
	timeCell->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	timeCell->setText(time);
	this->ui.TableWidget->setCellWidget(row, 4, timeCell);

	QDoubleSpinBox* tolDmdCell = new QDoubleSpinBox(this->ui.TableWidget);
	tolDmdCell->setDecimals(3);
	tolDmdCell->setMinimum(0);
	tolDmdCell->setMaximum(tolDmd + 1);
	tolDmdCell->setValue(tolDmd);
	tolDmdCell->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	tolDmdCell->setButtonSymbols(QAbstractSpinBox::NoButtons);
	this->ui.TableWidget->setCellWidget(row, 5, tolDmdCell);
	tolDmdCell->setReadOnly(true);  // not editable but can select
	tolDmdCell->setFrame(false);  // no frame

	QDoubleSpinBox* tolDlyCell = new QDoubleSpinBox(this->ui.TableWidget);
	tolDlyCell->setDecimals(3);
	tolDlyCell->setMinimum(0);
	tolDlyCell->setMaximum(tolDly + 1);
	tolDlyCell->setValue(tolDly);
	tolDlyCell->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	tolDlyCell->setButtonSymbols(QAbstractSpinBox::NoButtons);
	this->ui.TableWidget->setCellWidget(row, 6, tolDlyCell);
	tolDlyCell->setReadOnly(true);  // not editable but can select
	tolDlyCell->setFrame(false);  // no frame

	QDoubleSpinBox* avgDlyCell = new QDoubleSpinBox(this->ui.TableWidget);
	avgDlyCell->setDecimals(3);
	avgDlyCell->setMinimum(0);
	avgDlyCell->setMaximum(avgDly + 1);
	avgDlyCell->setValue(avgDly);
	avgDlyCell->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	avgDlyCell->setButtonSymbols(QAbstractSpinBox::NoButtons);
	this->ui.TableWidget->setCellWidget(row, 7, avgDlyCell);
	avgDlyCell->setReadOnly(true);  // not editable but can select
	avgDlyCell->setFrame(false);  // no frame

}


