#include "DCellPanel.h"

#include "Cell.h"

#include "DOkCancelButtons.h"
#include "GraphicsItems/DCellConnectorItem.h"
#include "Functions/Network/DCellEditFunction.h"
#include "Utils/MouseWheelWidgetAdjustmentGuard.h"

// --- --- --- --- --- Hint Texts --- --- --- --- ---

std::string hintTextEditCell{
"Some values are only editable via editing the Arc \r\n\
\r\n\
If this cell has multiple from or to cells\r\n\
You must ensure the merge/diverge ratios add up to 1\r\n\
Selecting a row in the merge/diverge table will highlight the corresponding from/to cell\r\n\
\r\n"
};

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DCellPanel::DCellPanel(Type_CellPanel type, const DISCO2_API::Cell* cell, 
	std::unordered_map<int, DCellConnectorItem*>&& toConnectors, std::unordered_map<int, DCellConnectorItem*>&& fromConnectors)
	: QWidget(nullptr), ref_cell(cell), ref_toConnectors(toConnectors), ref_fromConnectors(fromConnectors) {

	ui.setupUi(this);
	this->ui.SaturationFlow_doubleSpinBox->setMaximum(DBL_MAX);
	this->ui.JamDensity_doubleSpinBox->setMaximum(DBL_MAX);
	// NO MORE SCROLLING
	auto noMoreScrolling = new MouseWheelWidgetAdjustmentGuard{ this };
	this->ui.Length_doubleSpinBox->installEventFilter(noMoreScrolling);
	this->ui.FreeFlowSpeedLabel_doubleSpinBox->installEventFilter(noMoreScrolling);
	this->ui.BackwardShockwaveSpeed_doubleSpinBox->installEventFilter(noMoreScrolling);
	this->ui.JamDensity_doubleSpinBox->installEventFilter(noMoreScrolling);
	this->ui.SaturationFlow_doubleSpinBox->installEventFilter(noMoreScrolling);

	switch (type) {
	case Type_CellPanel::edit:
		ui.DCell_groupBox1->setTitle("Edit Cell");
		this->insertHintLabel(type);
		this->showCellIdAndParam(true);
		this->insertOkButtons();
		this->setEditable(true);
		break;
	case Type_CellPanel::view:
		ui.DCell_groupBox1->setTitle("Cell");
		this->insertHintLabel(type);
		this->showCellIdAndParam(false);
		this->setEditable(false);
		break;
	}

	//connections
	QObject::connect(
		this->ui.DivergeRatio_tableWidget->selectionModel(), &QItemSelectionModel::selectionChanged,
		this, &DCellPanel::divTableSelectionChanged);
	QObject::connect(
		this->ui.MergeRatio_tableWidget->selectionModel(), &QItemSelectionModel::selectionChanged,
		this, &DCellPanel::mrgTableSelectionChanged);
}

DCellPanel::~DCellPanel() {

}

// --- --- --- --- --- Settings for Functions --- --- --- --- ---

void DCellPanel::setEditable(bool editable) {
	//set all to read only mode
	this->ui.Name_LineEdit->setReadOnly(!editable);
	this->ui.Details_PlainTextEdit->setReadOnly(!editable);
	this->ui.BackwardShockwaveSpeed_doubleSpinBox->setReadOnly(!editable);
	this->ui.JamDensity_doubleSpinBox->setReadOnly(!editable);
	this->ui.SaturationFlow_doubleSpinBox->setReadOnly(!editable);
	if (editable) {
		//change the color to show params can't be edited
		QPalette palette = this->ui.FreeFlowSpeedLabel_doubleSpinBox->palette();
		palette.setColor(QPalette::ColorRole::Base, this->palette().color(QPalette::ColorRole::Background));
		this->ui.FreeFlowSpeedLabel_doubleSpinBox->setPalette(palette);
		this->ui.Length_doubleSpinBox->setPalette(palette);
		this->ui.NumberOfLane_spinBox->setPalette(palette);
	}

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

void DCellPanel::enableConfirmButton() {
	this->ok_Buttons->ref_confirmButton->setEnabled(true);
}

void DCellPanel::setupEditButtons(DCellEditFunction* function) {
	//set the text of the buttons
	this->ok_Buttons->ref_confirmButton->setText("Edit");
	this->ok_Buttons->ref_cancelButton->setText("Cancel");
	this->ok_Buttons->ref_confirmButton->setEnabled(false);

	//connect buttons to function methods
	QObject::connect(this->ok_Buttons->ref_confirmButton, &QPushButton::clicked, function, &DCellEditFunction::checkAndEditCell);
	QObject::connect(this->ok_Buttons->ref_cancelButton, &QPushButton::clicked, function, &DCellEditFunction::cancel);

	//connect internal value changes to the confirm button
	QObject::connect(this->ui.Name_LineEdit, &QLineEdit::textChanged, this, &DCellPanel::enableConfirmButton);
	QObject::connect(this->ui.Details_PlainTextEdit, &QPlainTextEdit::textChanged, this, &DCellPanel::enableConfirmButton);
	QObject::connect(this->ui.BackwardShockwaveSpeed_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DCellPanel::enableConfirmButton);
	QObject::connect(this->ui.JamDensity_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DCellPanel::enableConfirmButton);
	QObject::connect(this->ui.SaturationFlow_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DCellPanel::enableConfirmButton);
}

void DCellPanel::failMessage(QString&& msg) {
	this->ok_Buttons->ref_feedbackLabel->setText(msg);
	this->ok_Buttons->ref_feedbackLabel->update();
}

// --- --- --- --- --- Getters for relaying input form values --- --- --- --- ---

double DCellPanel::getQInput() {
	return this->ui.SaturationFlow_doubleSpinBox->value()/3600;
}

double DCellPanel::getKInput() {
	return this->ui.JamDensity_doubleSpinBox->value()/1000;
}

double DCellPanel::getWInput() {
	return this->ui.BackwardShockwaveSpeed_doubleSpinBox->value();
}

double DCellPanel::getVfInput() {
	return this->ui.FreeFlowSpeedLabel_doubleSpinBox->value();
}

QString DCellPanel::getNameInput() {
	return this->ui.Name_LineEdit->text();
}

QString DCellPanel::getDetailsInput() {
	return this->ui.Details_PlainTextEdit->toPlainText();
}

std::unordered_map<int, double> DCellPanel::getMrgTableInput() {
	std::unordered_map<int, double> out{};
	for (int i = 0; i < this->ui.MergeRatio_tableWidget->rowCount(); ++i)
		out.emplace(
			((QSpinBox*)(this->ui.MergeRatio_tableWidget->cellWidget(i, 0)))->value(),
			((QDoubleSpinBox*)(this->ui.MergeRatio_tableWidget->cellWidget(i, 1)))->value());
	return out;
}

std::unordered_map<int, double> DCellPanel::getDivTableInput() {
	std::unordered_map<int, double> out{};
	for (int i = 0; i < this->ui.DivergeRatio_tableWidget->rowCount(); ++i)
		out.emplace(
			((QSpinBox*)(this->ui.DivergeRatio_tableWidget->cellWidget(i, 0)))->value(),
			((QDoubleSpinBox*)(this->ui.DivergeRatio_tableWidget->cellWidget(i, 1)))->value());
	return out;
}

// --- --- --- --- --- Slots for connector highlight --- --- --- --- ---

void DCellPanel::mrgTableSelectionChanged() {
	for(auto& connectors : this->ref_fromConnectors)
		connectors.second->setSelected(false);

	for (auto& row : this->ui.MergeRatio_tableWidget->selectionModel()->selectedRows()) {
		int fromCellId = ((QSpinBox*)(this->ui.MergeRatio_tableWidget->cellWidget(row.row(), 0)))->value();
		if (this->ref_fromConnectors.count(fromCellId))
			this->ref_fromConnectors.at(fromCellId)->setSelected(true);
	}
}

void DCellPanel::divTableSelectionChanged() {
	for (auto& connectors : this->ref_toConnectors)
		connectors.second->setSelected(false);

	for (auto& row : this->ui.DivergeRatio_tableWidget->selectionModel()->selectedRows()) {
		int toCellId = ((QSpinBox*)(this->ui.DivergeRatio_tableWidget->cellWidget(row.row(), 0)))->value();
		if (this->ref_toConnectors.count(toCellId))
			this->ref_toConnectors.at(toCellId)->setSelected(true);
	}
}

// --- --- --- --- --- Private Utils --- --- --- --- ---

void DCellPanel::insertHintLabel(Type_CellPanel type) {
	this->ui.hint_label->setWordWrap(true);
	switch (type) {
	case Type_CellPanel::edit:
		this->ui.hint_label->setText(hintTextEditCell.c_str());
		break;
	}
}

void DCellPanel::insertOkButtons() {
	this->ok_Buttons = new DOkCancelButtons(this->ui.scrollAreaWidgetContents);
	this->ok_Buttons->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

	//always put the hint label at bottom before the spacer
	int numOfWidget = this->ui.verticalLayout_5->count();
	this->ui.verticalLayout_5->insertWidget(numOfWidget - 1, this->ok_Buttons);
}

void DCellPanel::showCellIdAndParam(bool editable) {
	//show the arc id
	this->ui.CellIdText_Label->setText("Cell ID");
	this->ui.CellId_Label->setText(std::to_string(this->ref_cell->getId()).c_str());
	//show name and details
	this->ui.Name_LineEdit->setText({ this->ref_cell->getVisInfo().getName().c_str() });
	this->ui.Details_PlainTextEdit->setPlainText({ this->ref_cell->getVisInfo().getDetails().c_str() });
	//show the existing param
	this->ui.Length_doubleSpinBox->setValue(this->ref_cell->getLength());
	this->ui.NumberOfLane_spinBox->setValue(this->ref_cell->getNumOfLanes());
	this->ui.SaturationFlow_doubleSpinBox->setValue(this->ref_cell->getFlowCapacityPerLane()*3600);
	this->ui.JamDensity_doubleSpinBox->setValue(this->ref_cell->getJamDensity()*1000);
	this->ui.FreeFlowSpeedLabel_doubleSpinBox->setValue(this->ref_cell->getFreeSpeed());
	this->ui.BackwardShockwaveSpeed_doubleSpinBox->setValue(this->ref_cell->getBackwardsWaveSpeed());
	
	//deal with mrg ratios
	if (this->ref_cell->getFromCellIds().size() > 1 && this->ref_fromConnectors.size() > 1) {
		this->ui.DCell_groupBox3->setVisible(true);
		this->ui.MergeRatio_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);

		for (auto& ratio : this->ref_cell->getFromCellIds())
			if (this->ref_fromConnectors.count(ratio.first))
				this->addRowToTable(this->ui.MergeRatio_tableWidget, ratio.first, ratio.second, !editable);
	}
	else
		this->ui.DCell_groupBox3->setVisible(false);
	//deal with div ratios
	if (this->ref_cell->getToCellIds().size() > 1 && this->ref_toConnectors.size() > 1) {
		this->ui.DCell_groupBox2->setVisible(true);
		this->ui.DivergeRatio_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);

		for (auto& ratio : this->ref_cell->getToCellIds())
			if (this->ref_toConnectors.count(ratio.first))
				this->addRowToTable(this->ui.DivergeRatio_tableWidget, ratio.first, ratio.second, !editable);
	}
	else
		this->ui.DCell_groupBox2->setVisible(false);

}

void DCellPanel::addRowToTable(QTableWidget* table, int cellId, double ratio, bool isReadOnly) {
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
	if(!isReadOnly)
		QObject::connect(ratioCell, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DCellPanel::enableConfirmButton);

}


