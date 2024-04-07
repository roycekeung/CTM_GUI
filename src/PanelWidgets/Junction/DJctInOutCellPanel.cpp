#include "DJctInOutCellPanel.h"

#include "Cell.h"
#include "Cell_DemandSink.h"
#include "Jct_Node.h"
#include "Jct_Link.h"

#include "DOkCancelButtons.h"
#include "GraphicsItems/DCellConnectorItem.h"
#include "GraphicsItems/DPseudoJctLinkItem.h"
#include "Utils/MouseWheelWidgetAdjustmentGuard.h"

#include "Functions/Junction/DJctInOutCellEditFunction.h"

// --- --- --- --- --- Hint Texts --- --- --- --- ---

const std::string DJctInOutCellPanel::hintTextEditCell{
"If this cell has multiple from or to Junction Links\r\n\
You must ensure the merge / diverge ratios add up to 1\r\n\
Selecting a row in the merge / diverge table will highlight the corresponding from / to Junction Link\r\n\
\r\n"
};

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DJctInOutCellPanel::DJctInOutCellPanel(Type_CellPanel type, const DISCO2_API::Cell* cell, const DISCO2_API::Jct_Node& jctNode,
	std::unordered_map<int, DPseudoJctLinkItem*>&& jctLinks)
	: QWidget(nullptr), ref_cell(cell), ref_jctNode(jctNode), ref_jctLinks(jctLinks) {

	ui.setupUi(this);
	this->ui.SaturationFlow_doubleSpinBox->setMaximum(DBL_MAX);
	this->ui.JamDensity_doubleSpinBox->setMaximum(DBL_MAX);

	switch (type) {
	case Type_CellPanel::edit:
		ui.DCell_groupBox1->setTitle("Edit In/Out Cell");
		this->insertHintLabel(type);
		this->showCellIdAndParam(true);
		this->insertOkButtons();
		this->setEditable(true);
		break;
	case Type_CellPanel::view:
		ui.DCell_groupBox1->setTitle("In/Out Cell");
		this->insertHintLabel(type);
		this->showCellIdAndParam(false);
		this->setEditable(false);
		break;
	}

	//connections
	QObject::connect(
		this->ui.DivergeRatio_tableWidget->selectionModel(), &QItemSelectionModel::selectionChanged,
		this, &DJctInOutCellPanel::divTableSelectionChanged);
	QObject::connect(
		this->ui.MergeRatio_tableWidget->selectionModel(), &QItemSelectionModel::selectionChanged,
		this, &DJctInOutCellPanel::mrgTableSelectionChanged);
}

DJctInOutCellPanel::~DJctInOutCellPanel() {

}

// --- --- --- --- --- Settings for Functions --- --- --- --- ---

void DJctInOutCellPanel::setEditable(bool editable) {
	//setEditable for tables
	if (editable) {
		this->ui.DivergeRatio_tableWidget->setEditTriggers(QAbstractItemView::EditTrigger::DoubleClicked | QAbstractItemView::EditTrigger::AnyKeyPressed);
		this->ui.DivergeRatio_tableWidget->setEditTriggers(QAbstractItemView::EditTrigger::DoubleClicked | QAbstractItemView::EditTrigger::AnyKeyPressed);
	}
	else {
		this->ui.DivergeRatio_tableWidget->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
		this->ui.DivergeRatio_tableWidget->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
	}
}

void DJctInOutCellPanel::enableConfirmButton() {
	this->ok_Buttons->ref_confirmButton->setEnabled(true);
}

void DJctInOutCellPanel::setupEditButtons(DJctInOutCellEditFunction* function) {
	//set the text of the buttons
	this->ok_Buttons->ref_confirmButton->setText("Edit");
	this->ok_Buttons->ref_cancelButton->setText("Cancel");
	this->ok_Buttons->ref_confirmButton->setEnabled(false);

	//connect buttons to function methods
	QObject::connect(this->ok_Buttons->ref_confirmButton, &QPushButton::clicked, function, &DJctInOutCellEditFunction::checkAndEditCell);
	QObject::connect(this->ok_Buttons->ref_cancelButton, &QPushButton::clicked, function, &DJctInOutCellEditFunction::cancel);
}

void DJctInOutCellPanel::failMessage(QString&& msg) {
	this->ok_Buttons->ref_feedbackLabel->setText(msg);
	this->ok_Buttons->ref_feedbackLabel->update();
}

// --- --- --- --- --- Getters for relaying input form values --- --- --- --- ---

std::unordered_map<int, double> DJctInOutCellPanel::getMrgTableInput() {
	std::unordered_map<int, double> out{};
	for (int i = 0; i < this->ui.MergeRatio_tableWidget->rowCount(); ++i)
		out.emplace(
			((QSpinBox*)(this->ui.MergeRatio_tableWidget->cellWidget(i, 0)))->value(),
			((QDoubleSpinBox*)(this->ui.MergeRatio_tableWidget->cellWidget(i, 1)))->value());
	return out;
}

std::unordered_map<int, double> DJctInOutCellPanel::getDivTableInput() {
	std::unordered_map<int, double> out{};
	for (int i = 0; i < this->ui.DivergeRatio_tableWidget->rowCount(); ++i)
		out.emplace(
			((QSpinBox*)(this->ui.DivergeRatio_tableWidget->cellWidget(i, 0)))->value(),
			((QDoubleSpinBox*)(this->ui.DivergeRatio_tableWidget->cellWidget(i, 1)))->value());
	return out;
}

// --- --- --- --- --- Slots for connector highlight --- --- --- --- ---

void DJctInOutCellPanel::mrgTableSelectionChanged() {
	for (auto& connectors : this->ref_jctLinks)
		connectors.second->setSelected(false);

	for (auto& row : this->ui.MergeRatio_tableWidget->selectionModel()->selectedRows()) {
		int fromCellId = ((QSpinBox*)(this->ui.MergeRatio_tableWidget->cellWidget(row.row(), 0)))->value();
		if (this->ref_jctLinks.count(fromCellId))
			this->ref_jctLinks.at(fromCellId)->setSelected(true);
	}
}

void DJctInOutCellPanel::divTableSelectionChanged() {
	for (auto& connectors : this->ref_jctLinks)
		connectors.second->setSelected(false);

	for (auto& row : this->ui.DivergeRatio_tableWidget->selectionModel()->selectedRows()) {
		int toCellId = ((QSpinBox*)(this->ui.DivergeRatio_tableWidget->cellWidget(row.row(), 0)))->value();
		if (this->ref_jctLinks.count(toCellId))
			this->ref_jctLinks.at(toCellId)->setSelected(true);
	}
}

// --- --- --- --- --- Private Utils --- --- --- --- ---

void DJctInOutCellPanel::insertHintLabel(Type_CellPanel type) {
	this->hint_Label = new QLabel(this->ui.DCell_groupBox1);
	this->hint_Label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	this->hint_Label->setWordWrap(true);

	switch (type) {
	case Type_CellPanel::edit:
		this->hint_Label->setText(hintTextEditCell.c_str());
		break;
	}

	//always put the hint label at top
	int numOfWidget = this->ui.verticalLayout_5->count();
	this->ui.verticalLayout_5->insertWidget(0, this->hint_Label);
}

void DJctInOutCellPanel::insertOkButtons() {
	this->ok_Buttons = new DOkCancelButtons(this->ui.DCell_groupBox1);
	this->ok_Buttons->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

	//always put the hint label at bottom before the spacer
	int numOfWidget = this->ui.verticalLayout_5->count();
	this->ui.verticalLayout_5->insertWidget(numOfWidget - 1, this->ok_Buttons);
}

void DJctInOutCellPanel::showCellIdAndParam(bool editable) {
	//show the arc id
	this->ui.CellIdText_Label->setText("Cell Id");
	this->ui.CellId_Label->setText(std::to_string(this->ref_cell->getId()).c_str());
	//show name and details
	this->ui.Name_LineEdit->setText({ this->ref_cell->getVisInfo().getName().c_str() });
	this->ui.Details_PlainTextEdit->setPlainText({ this->ref_cell->getVisInfo().getDetails().c_str() });

	const DISCO2_API::Cell_DemandSink* snkCellPtr = dynamic_cast<const DISCO2_API::Cell_DemandSink*>(this->ref_cell);
	if (snkCellPtr) {	//only sink cell can be connected to jct
		//rename group bax
		this->ui.DCell_groupBox1->setTitle("Sink Cell Properties");
		//set the value
		this->ui.SaturationFlow_doubleSpinBox->setValue(snkCellPtr->getFlowInCapacity()*3600);

		//hide all the normal cell stuff
		this->ui.Length_Label->setVisible(false);
		this->ui.NumberOfLane_Label->setVisible(false);
		this->ui.JamDensity_Label->setVisible(false);
		this->ui.FreeFlowSpeed_Label->setVisible(false);
		this->ui.BackwardShockwaveSpeed_Label->setVisible(false);

		this->ui.Length_doubleSpinBox->setVisible(false);
		this->ui.NumberOfLane_spinBox->setVisible(false);
		this->ui.JamDensity_doubleSpinBox->setVisible(false);
		this->ui.FreeFlowSpeedLabel_doubleSpinBox->setVisible(false);
		this->ui.BackwardShockwaveSpeed_doubleSpinBox->setVisible(false);

		this->ui.unitL_label->setVisible(false);
		this->ui.unitKj_label->setVisible(false);
		this->ui.unitV_label->setVisible(false);
		this->ui.unitW_label->setVisible(false);
	}
	else {
		//show the existing param
		this->ui.Length_doubleSpinBox->setValue(this->ref_cell->getLength());
		this->ui.NumberOfLane_spinBox->setValue(this->ref_cell->getNumOfLanes());
		this->ui.SaturationFlow_doubleSpinBox->setValue(this->ref_cell->getFlowCapacityPerLane()*3600);
		this->ui.JamDensity_doubleSpinBox->setValue(this->ref_cell->getJamDensity()*1000);
		this->ui.FreeFlowSpeedLabel_doubleSpinBox->setValue(this->ref_cell->getFreeSpeed());
		this->ui.BackwardShockwaveSpeed_doubleSpinBox->setValue(this->ref_cell->getBackwardsWaveSpeed());

		//change the color to show params can't be edited
		QPalette palette = this->ui.FreeFlowSpeedLabel_doubleSpinBox->palette();
		palette.setColor(QPalette::ColorRole::Base, this->palette().color(QPalette::ColorRole::Background));
		this->ui.Name_LineEdit->setPalette(palette);
		this->ui.Details_PlainTextEdit->setPalette(palette);
		this->ui.Length_doubleSpinBox->setPalette(palette);
		this->ui.NumberOfLane_spinBox->setPalette(palette);
		this->ui.SaturationFlow_doubleSpinBox->setPalette(palette);
		this->ui.JamDensity_doubleSpinBox->setPalette(palette);
		this->ui.FreeFlowSpeedLabel_doubleSpinBox->setPalette(palette);
		this->ui.BackwardShockwaveSpeed_doubleSpinBox->setPalette(palette);
	}

	//deal with mrg ratios
	if (this->ref_jctNode.getOutCells().count(this->ref_cell->getId()) && this->ref_cell->getFromCellIds().size() > 1) {
		this->ui.DCell_groupBox3->setVisible(true);

		auto& ratioRef = this->ref_cell->getFromCellIds();
		for (auto& linkId : this->ref_jctNode.getConnectedLinkIds(this->ref_cell->getId())) {
			int fromCellId = this->ref_jctNode.getJctLink(linkId)->getArcEndCellId();
			if (this->ref_jctLinks.count(linkId) && ratioRef.count(fromCellId))
				this->addRowToTable(this->ui.MergeRatio_tableWidget, linkId, ratioRef.at(fromCellId), !editable);
		}
	}
	else
		this->ui.DCell_groupBox3->setVisible(false);
	//deal with div ratios
	if (this->ref_jctNode.getInCells().count(this->ref_cell->getId()) && this->ref_cell->getToCellIds().size() > 1) {
		this->ui.DCell_groupBox2->setVisible(true);

		auto& ratioRef = this->ref_cell->getToCellIds();
		for (auto& linkId : this->ref_jctNode.getConnectedLinkIds(this->ref_cell->getId())) {
			int toCellId = this->ref_jctNode.getJctLink(linkId)->getArcStartCellId();
			if (this->ref_jctLinks.count(linkId) && ratioRef.count(toCellId))
				this->addRowToTable(this->ui.DivergeRatio_tableWidget, linkId, ratioRef.at(toCellId), !editable);
		}
	}
	else
		this->ui.DCell_groupBox2->setVisible(false);

}

void DJctInOutCellPanel::addRowToTable(QTableWidget* table, int cellId, double ratio, bool isReadOnly) {
	int row = table->rowCount();
	table->insertRow(row);

	// cellId set up
	QSpinBox* idCell = new QSpinBox(table);
	idCell->setMaximum(INT_MAX);
	idCell->setValue(cellId);
	idCell->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	idCell->setButtonSymbols(QAbstractSpinBox::NoButtons);
	table->setCellWidget(row, 0, idCell);
	idCell->setReadOnly(true);
	idCell->setFrame(false);  // no frame

	// ratio set up
	QDoubleSpinBox* ratioCell = new QDoubleSpinBox(table);
	ratioCell->setDecimals(3);
	ratioCell->setMinimum(0);
	ratioCell->setMaximum(1);
	ratioCell->setValue(ratio);
	ratioCell->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	ratioCell->setButtonSymbols(QAbstractSpinBox::NoButtons);
	table->setCellWidget(row, 1, ratioCell);
	ratioCell->setReadOnly(isReadOnly);  // not editable but can select
	ratioCell->setFrame(false);  // no frame
	ratioCell->installEventFilter(new MouseWheelWidgetAdjustmentGuard(ratioCell));
	if (!isReadOnly)
		QObject::connect(ratioCell, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DJctInOutCellPanel::enableConfirmButton);
}