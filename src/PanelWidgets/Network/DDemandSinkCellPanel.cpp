#include "DDemandSinkCellPanel.h"

//DISCO GUI stuff
#include "../Functions/Network/DDemandSinkCellCreateFunc.h"
#include "../Functions/Network/DDemandSinkCellEditFunc.h"
#include "../Functions/Network/DDemandSinkCellViewFunc.h"
#include "Utils/MouseWheelWidgetAdjustmentGuard.h"
#include "Utils/DDefaultParamData.h"
#include "DOkCancelButtons.h"
#include "DAddRemoveButtons.h"
#include "GraphicsItems/DCellConnectorItem.h"
#include "GraphicsItems/DDemandSinkCellItem.h"

//Qt lib
#include <QString>
#include <QtWidgets/QLabel>

// std lib
#include <string>

//DISCO Core lib
#include "Cell_DemandSink.h"
#include "Dmd_Container.h"

// --- --- --- --- --- Hint Texts --- --- --- --- ---
std::string hintTextCreate{
"Left click on the map to select the location of the cell\r\n\
Click and drag the cell to reposition the cell\r\n\
Click and drag the corners to adjust the visual size of the cell\r\n\
Click and drag the top yellow point to rotate the cell\r\n\
\r\n"
};

std::string hintTextEdit{
"Click and drag the cell to reposition the cell\r\n\
Click and drag the corners to adjust the visual size of the cell\r\n\
Click and drag the top yellow point to rotate the cell\r\n\
\r\n\
A demand set must be first created in order to input a demand interval\r\n\
\r\n"
};

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DDemandSinkCellPanel::DDemandSinkCellPanel(Type_Panel typeOfPanel, DefaultParamDataSet* dparam,
	CellType typeOfCell, const DISCO2_API::Cell_DemandSink* DmdSnkCell,
	std::unordered_map<int, DCellConnectorItem*>&& toConnectors,
	std::unordered_map<int, DCellConnectorItem*>&& fromConnectors) :
	QWidget(nullptr), typeOfPanel(typeOfPanel), typeOfCell(typeOfCell), 
	ref_DmdSnkCell(DmdSnkCell), 
	ref_toConnectors(toConnectors), ref_fromConnectors(fromConnectors),
	ref_dParam(dparam){

	ui.setupUi(this);
	this->ui.FlowCap_doubleSpinBox->setMaximum(DBL_MAX);
	// NO MORE SCROLLING
	auto noMoreScrolling = new MouseWheelWidgetAdjustmentGuard{ this };
	this->ui.FlowCap_doubleSpinBox->installEventFilter(noMoreScrolling);

	switch (typeOfPanel) {
	case Type_Panel::create: {
		ui.ID_Label->setText("");
		ui.Property_groupBox->setTitle("Create");
		this->insertHintLabel(typeOfPanel);

		// connect the type of cell combobox in order to change the following panel visual
		QObject::connect(this->ui.Type_comboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DDemandSinkCellPanel::switchPanelUIByCelltype);

		//QHeaderView will automatically resize the section to fill the available space horizontally, fixed vertically. 
		ui.DemandInput_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
		/// Set row highlight when selected
		ui.DemandInput_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

		setTablesVisibleByCelltypeAndMode(typeOfPanel, typeOfCell);

		this->showCellIdAndDefaultParam();
		this->insertOkButtons();
		this->setEditable(true);	//lock input initialy
		break;
	}
	case Type_Panel::edit: {
		ui.Property_groupBox->setTitle("Edit");
		this->insertHintLabel(typeOfPanel);

		//QHeaderView will automatically resize the section to fill the available space horizontally, fixed vertically. 
		ui.DemandInput_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
		ui.DivergeRatioInput_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
		ui.MergeRatioInput_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
		/// Set row highlight when selected
		ui.DemandInput_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
		ui.DivergeRatioInput_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
		ui.MergeRatioInput_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
		//// turn off the row header of qtablewidget 
		//ui.DemandInput_tableWidget->verticalHeader()->hide();
		//ui.DivergeRatioInput_tableWidget->verticalHeader()->hide();
		//ui.MergeRatioInput_tableWidget->verticalHeader()->hide();

		setTablesVisibleByCelltypeAndMode(typeOfPanel, typeOfCell);

		insertAddRemoveButtons(typeOfPanel);

		this->showCellIdAndParam(true);
		this->insertOkButtons();
		this->setEditable(true);
		break;
	}
	case Type_Panel::view: {
		ui.Property_groupBox->setTitle("View property");
		this->insertHintLabel(typeOfPanel);

		//QHeaderView will automatically resize the section to fill the available space horizontally, fixed vertically. 
		ui.DemandInput_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
		ui.DivergeRatioInput_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
		ui.MergeRatioInput_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
		/// Set row highlight when selected
		ui.DemandInput_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
		ui.DivergeRatioInput_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
		ui.MergeRatioInput_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
		//// turn off the row header of qtablewidget 
		//ui.DemandInput_tableWidget->verticalHeader()->hide();
		//ui.DivergeRatioInput_tableWidget->verticalHeader()->hide();
		//ui.MergeRatioInput_tableWidget->verticalHeader()->hide();

		setTablesVisibleByCelltypeAndMode(typeOfPanel, typeOfCell);

		this->showCellIdAndParam(false);
		this->setEditable(false);
		break;
	}
	}// end of Type_Panel  swtich

	// connections for the diverge and merge table
	QObject::connect(
		this->ui.DivergeRatioInput_tableWidget->selectionModel(), &QItemSelectionModel::selectionChanged,
		this, &DDemandSinkCellPanel::divTableSelectionChanged);
	QObject::connect(
		this->ui.MergeRatioInput_tableWidget->selectionModel(), &QItemSelectionModel::selectionChanged,
		this, &DDemandSinkCellPanel::mrgTableSelectionChanged);
}

DDemandSinkCellPanel::~DDemandSinkCellPanel() {}

void DDemandSinkCellPanel::connectCreateFunction(DDemandSinkCellCreateFunc* function) {
	//set the text of the buttons
	this->ok_Buttons->ref_confirmButton->setText("Create");
	this->ok_Buttons->ref_cancelButton->setText("Cancel");
	this->ok_Buttons->ref_confirmButton->setEnabled(false);

	//connect buttons to function methods
	QObject::connect(this->ok_Buttons->ref_confirmButton, &QPushButton::clicked, function, &DDemandSinkCellCreateFunc::checkAndCreate);
	QObject::connect(this->ok_Buttons->ref_cancelButton, &QPushButton::clicked, function, &DDemandSinkCellCreateFunc::cancel);

	//connect internal changes to confirm button
	QObject::connect(ui.Name_lineEdit, &QLineEdit::textChanged, this, &DDemandSinkCellPanel::enableConfirmButton);
	QObject::connect(ui.Type_comboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DDemandSinkCellPanel::enableConfirmButton);
	QObject::connect(ui.Details_textEdit, &QTextEdit::textChanged, this, &DDemandSinkCellPanel::enableConfirmButton);
	QObject::connect(ui.FlowCap_doubleSpinBox, &QDoubleSpinBox::textChanged, this, &DDemandSinkCellPanel::enableConfirmButton);
	QObject::connect(ui.DemandInput_tableWidget, &QTableWidget::itemSelectionChanged, this, &DDemandSinkCellPanel::enableConfirmButton);
	QObject::connect(ui.DivergeRatioInput_tableWidget, &QTableWidget::itemSelectionChanged, this, &DDemandSinkCellPanel::enableConfirmButton);
	QObject::connect(ui.MergeRatioInput_tableWidget, &QTableWidget::itemSelectionChanged, this, &DDemandSinkCellPanel::enableConfirmButton);
}

void DDemandSinkCellPanel::connectEditFunction(DDemandSinkCellEditFunc* function) {
	if (DemandInput_AddRemoveButtons) {
		//connect buttons to function methods
		QObject::connect(this->DemandInput_AddRemoveButtons->AddButton, &QPushButton::clicked, function, &DDemandSinkCellEditFunc::addEmptyRow);
		QObject::connect(this->DemandInput_AddRemoveButtons->RemoveButton, &QPushButton::clicked, function, &DDemandSinkCellEditFunc::removerow);
	}

	//set the text of the buttons
	this->ok_Buttons->ref_confirmButton->setText("Edit");
	this->ok_Buttons->ref_cancelButton->setText("Cancel");
	this->ok_Buttons->ref_confirmButton->setEnabled(false);

	//connect buttons to function methods
	QObject::connect(this->ok_Buttons->ref_confirmButton, &QPushButton::clicked, function, &DDemandSinkCellEditFunc::checkAndEdit);
	QObject::connect(this->ok_Buttons->ref_cancelButton, &QPushButton::clicked, function, &DDemandSinkCellEditFunc::cancel);

	//connect internal changes to confirm button
	QObject::connect(ui.Name_lineEdit, &QLineEdit::textChanged, this, &DDemandSinkCellPanel::enableConfirmButton);
	QObject::connect(ui.Type_comboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DDemandSinkCellPanel::enableConfirmButton);
	QObject::connect(ui.Details_textEdit, &QTextEdit::textChanged, this, &DDemandSinkCellPanel::enableConfirmButton);
	QObject::connect(ui.FlowCap_doubleSpinBox, &QDoubleSpinBox::textChanged, this, &DDemandSinkCellPanel::enableConfirmButton);
	QObject::connect(ui.DemandInput_tableWidget, &QTableWidget::itemSelectionChanged, this, &DDemandSinkCellPanel::enableConfirmButton);
	QObject::connect(ui.DivergeRatioInput_tableWidget, &QTableWidget::itemSelectionChanged, this, &DDemandSinkCellPanel::enableConfirmButton);
	QObject::connect(ui.MergeRatioInput_tableWidget, &QTableWidget::itemSelectionChanged, this, &DDemandSinkCellPanel::enableConfirmButton);

	// connect the type of cell combobox in order to change the following panel visual
	QObject::connect(ui.Plan_comboBox, static_cast<void(QComboBox::*)(const QString&)>(&QComboBox::currentTextChanged), function, &DDemandSinkCellEditFunc::updateDemandInputTablefromCore);

	// connect the DDemandSinkCellItem in order to enableConfirmButton
	QObject::connect(function, &DDemandSinkCellEditFunc::SendVisibleSignalToAddRemoveButton, this, &DDemandSinkCellPanel::setVisibleAddRemoveButtons);

}

void DDemandSinkCellPanel::connectViewFunction(DDemandSinkCellViewFunc* function) {
	// connect the type of cell combobox in order to change the following panel visual
	QObject::connect(ui.Plan_comboBox, static_cast<void(QComboBox::*)(const QString&)>(&QComboBox::currentTextChanged), function, &DDemandSinkCellViewFunc::updateDemandInputTablefromCore);

}

// --- --- --- --- --- Settings for Functions --- --- --- --- ---
void DDemandSinkCellPanel::setEditable(bool editable) {
	//set all to read only mode
	this->ui.Name_lineEdit->setReadOnly(!editable);
	this->ui.Details_textEdit->setReadOnly(!editable);
	this->ui.Type_comboBox->setEnabled(this->typeOfPanel == Type_Panel::create ? true : false);
	this->ui.FlowCap_doubleSpinBox->setReadOnly(!editable);

	//remove the up down buttons
	this->ui.FlowCap_doubleSpinBox->setButtonSymbols(editable ? QAbstractSpinBox::UpDownArrows : QAbstractSpinBox::NoButtons);
}

void DDemandSinkCellPanel::switchPanelUIByCelltype(int CellType_index) {
	// only activate in create mode
	if (CellType_index == 0) {
		// Demand cell
		setTablesVisibleByCelltypeAndMode(this->typeOfPanel, DDemandSinkCellPanel::CellType::Demand);
	}
	else if (CellType_index ==1) {
		// Sink cell
		setTablesVisibleByCelltypeAndMode(this->typeOfPanel, DDemandSinkCellPanel::CellType::Sink);
	}
	this->update();
}

void DDemandSinkCellPanel::setTablesVisibleByCelltypeAndMode(Type_Panel typeOfPanel, CellType typeOfCell) {
	switch (typeOfPanel) {
	case Type_Panel::create: {

		if (typeOfCell == DDemandSinkCellPanel::CellType::Demand) {
			// Demand cell
			// dont have DivergeRatioInput_tableWidget and MergeRatioInput_tableWidget; 
			// DemandInput_tableWidget is hard to define at firstly created whereas the dmd_set dont initiate the loader 
			// eventhough u have newly created a dmd set, so that any continuous (uniform) or discrete (non-uniform) loading cannot be added
			ui.Property_groupBox->setTitle("Create Demand Cell");
			ui.DemandInput_groupBox->setVisible(false);  
			ui.DivergeRatioInput_groupBox->setVisible(false);
			ui.MergeRatioInput_groupBox->setVisible(false);
		}
		else if (typeOfCell == DDemandSinkCellPanel::CellType::Sink) {
			// Sink cell
			ui.Property_groupBox->setTitle("Create Sink Cell");
			ui.DemandInput_groupBox->setVisible(false);
			ui.DivergeRatioInput_groupBox->setVisible(false);
			ui.MergeRatioInput_groupBox->setVisible(false);
		}// end of typeOfCell if loop
		break;
	}
	case Type_Panel::edit: {

		if (typeOfCell == DDemandSinkCellPanel::CellType::Demand) {
			// Demand cell
			ui.DemandInput_groupBox->setVisible(true);
		}
		else if (typeOfCell == DDemandSinkCellPanel::CellType::Sink) {
			// Sink cell
			ui.DemandInput_groupBox->setVisible(false);
		}// end of typeOfCell if loop
		break;
	}
	case Type_Panel::view: {

		if (typeOfCell == DDemandSinkCellPanel::CellType::Demand) {
			// Demand cell
			ui.DemandInput_groupBox->setVisible(true);
		}
		else if (typeOfCell == DDemandSinkCellPanel::CellType::Sink) {
			// Sink cell
			ui.DemandInput_groupBox->setVisible(false);
		}// end of typeOfCell if loop
		break;
	}
	} // end of Type_Panel swtich
}


void DDemandSinkCellPanel::insertHintLabel(DDemandSinkCellPanel::Type_Panel typeOfPanel) {
	this->hint_Label = new QLabel(this->ui.Property_groupBox);
	this->hint_Label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	this->hint_Label->setWordWrap(true);

	switch (typeOfPanel) {
	case Type_Panel::create:
		this->hint_Label->setText(hintTextCreate.c_str());
		break;
	case Type_Panel::edit:
		this->hint_Label->setText(hintTextEdit.c_str());
		break;
	}

	//always put the hint label at top
	int numOfWidget = this->ui.verticalLayout->count();
	this->ui.verticalLayout->insertWidget(0, this->hint_Label);
}

// insert external button 
void DDemandSinkCellPanel::insertAddRemoveButtons(Type_Panel typeOfPanel) {
	if (typeOfPanel == Type_Panel::edit) {
		int numOfWidget;
		
		this->DemandInput_AddRemoveButtons = new DAddRemoveButtons(this->ui.DemandInput_tableWidget);
		this->DemandInput_AddRemoveButtons->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
		// DemandInput_verticalLayout wrap up the Demand Input Table
		numOfWidget = this->ui.DemandInput_verticalLayout->count();
		this->ui.DemandInput_verticalLayout->insertWidget(numOfWidget, this->DemandInput_AddRemoveButtons);
	}
	// else create mode, DemandInput table cannot write into the core yet so disable add remove actions
	// else view mode, dont have the add delete row options
}

void DDemandSinkCellPanel::insertOkButtons() {
	this->ok_Buttons = new DOkCancelButtons(this->ui.Property_groupBox);
	this->ok_Buttons->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	//always put the okbutton at bottom after the spacer
	int numOfWidget = this->ui.verticalLayout->count();
	this->ui.verticalLayout->insertWidget(numOfWidget, this->ok_Buttons);
}

// --- --- --- --- --- Private Utils --- --- --- --- ---
void DDemandSinkCellPanel::showCellIdAndDefaultParam() {
	//fetch the default params from somewhere
	// empty pointer of DefaultParamDataSet would not set any values into the panel
	if (!ref_dParam) {
		return;
	}

	// save newly amended inputted params to default data struct bakcup
	if (this->ref_dParam->isShowLastInput && this->ref_dParam->m_DmdCellFlowParams.SaturationFlow != -1) {
		// true, show DmdCellFlow param
		this->ui.FlowCap_doubleSpinBox->setValue(ref_dParam->m_DmdCellFlowParams.SaturationFlow*3600);
	}
	else {
		// show flow param
		this->ui.FlowCap_doubleSpinBox->setValue(ref_dParam->m_FlowParams.SaturationFlow*3600);
	}
}

void DDemandSinkCellPanel::showCellIdAndParam(bool editable) {
	//show name and details and type
	this->ui.Name_lineEdit->setText({ this->ref_DmdSnkCell->getVisInfo().getName().c_str() });
	this->ui.Details_textEdit->setPlainText({ this->ref_DmdSnkCell->getVisInfo().getDetails().c_str() });

	// enum CellType { Demand =0 , Sink =1};
	// getFlowInCapacity == 0  means no flow in, does flow out; thus demand cell;  Type="DmdSnk" InQ="0"
	// getFlowInCapacity != 0  means does flow in, no flow out; thus Sink cell;   Type="DmdSnk" OutQ="0"
	if (this->ref_DmdSnkCell->getFlowInCapacity() == 0) {
		// Demand Cell 

		// show the cell id
		this->ui.IDText_Label->setText("Demand Cell ID");
		// show cell type
		this->ui.Type_comboBox->setCurrentIndex(0);
		// show existing param
		this->ui.FlowCap_doubleSpinBox->setValue(this->ref_DmdSnkCell->getFlowCapacity() * 3600 * 1000 / this->ref_DmdSnkCell->getTimeStepSize());   // this->m_Q = outQ; get outQ (flow out)
	}
	else {
		// Sink Cell 

		// show the cell id
		this->ui.IDText_Label->setText("Sink Cell ID");
		// show cell type
		this->ui.Type_comboBox->setCurrentIndex(1);
		// show existing param
		this->ui.FlowCap_doubleSpinBox->setValue(this->ref_DmdSnkCell->getFlowInCapacity() * 3600 * 1000 / this->ref_DmdSnkCell->getTimeStepSize());  // this->m_inQ = inQ; get inQ (flow in)

	}
	// show the cell id
	this->ui.ID_Label->setText(std::to_string(this->ref_DmdSnkCell->getId()).c_str());

	//deal with mrg ratios
	if (this->ref_DmdSnkCell->getFromCellIds().size() > 1 && this->ref_fromConnectors.size() > 1) {
		this->ui.MergeRatioInput_groupBox->setVisible(true);

		for (auto& ratio : this->ref_DmdSnkCell->getFromCellIds())
			if (this->ref_fromConnectors.count(ratio.first))
				this->addRowToTable(this->ui.MergeRatioInput_tableWidget, ratio.first, ratio.second, !editable);
	}
	else
		this->ui.MergeRatioInput_groupBox->setVisible(false);
	//deal with div ratios
	if (this->ref_DmdSnkCell->getToCellIds().size() > 1 && this->ref_toConnectors.size() > 1) {
		this->ui.DivergeRatioInput_groupBox->setVisible(true);

		for (auto& ratio : this->ref_DmdSnkCell->getToCellIds())
			if (this->ref_toConnectors.count(ratio.first))
				this->addRowToTable(this->ui.DivergeRatioInput_tableWidget, ratio.first, ratio.second, !editable);
	}
	else
		this->ui.DivergeRatioInput_groupBox->setVisible(false);

}

// for merge diverge table
void DDemandSinkCellPanel::addRowToTable(QTableWidget* table, int cellId, double ratio, bool isReadOnly) {
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
	ratioCell->setDecimals(2);
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
		QObject::connect(ratioCell, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DDemandSinkCellPanel::enableConfirmButton);

}

// --- --- --- --- --- Settings for Functions --- --- --- --- ---

void DDemandSinkCellPanel::connectItemToConfirmButton(DDemandSinkCellItem* Item) {
	// connect the DDemandSinkCellItem in order to enableConfirmButton
	QObject::connect(Item, &DDemandSinkCellItem::ShapePosChange, this, &DDemandSinkCellPanel::enableConfirmButton);
}

// --- --- --- --- --- show error tool --- --- --- --- --- 
void DDemandSinkCellPanel::failMessage(QString&& msg) {
	this->ok_Buttons->ref_feedbackLabel->setText(msg);
	this->ok_Buttons->ref_feedbackLabel->update();
}

// --- --- --- --- --- Getters for relaying input form values --- --- --- --- ---
QString DDemandSinkCellPanel::getNameInput() {
	return this->ui.Name_lineEdit->text();
}

int DDemandSinkCellPanel::getCellTypeInput() {
	// enum CellType { Demand =0 , Sink =1};
	return this->ui.Type_comboBox->currentIndex();
}

QString DDemandSinkCellPanel::getDetailsInput() {
	return this->ui.Details_textEdit->toPlainText();
}

double DDemandSinkCellPanel::getFlowCapInput() {
	return this->ui.FlowCap_doubleSpinBox->value()/3600;
}

Ui::DDemandSinkCellPanel DDemandSinkCellPanel::getUI() {
	return this->ui;
}

std::unordered_map<int, double> DDemandSinkCellPanel::getMrgTableInput() {
	std::unordered_map<int, double> out{};
	for (int i = 0; i < this->ui.MergeRatioInput_tableWidget->rowCount(); ++i)
		out.emplace(
			((QSpinBox*)(this->ui.MergeRatioInput_tableWidget->cellWidget(i, 0)))->value(),
			((QDoubleSpinBox*)(this->ui.MergeRatioInput_tableWidget->cellWidget(i, 1)))->value());
	return out;
}

std::unordered_map<int, double> DDemandSinkCellPanel::getDivTableInput() {
	std::unordered_map<int, double> out{};
	for (int i = 0; i < this->ui.DivergeRatioInput_tableWidget->rowCount(); ++i)
		out.emplace(
			((QSpinBox*)(this->ui.DivergeRatioInput_tableWidget->cellWidget(i, 0)))->value(),
			((QDoubleSpinBox*)(this->ui.DivergeRatioInput_tableWidget->cellWidget(i, 1)))->value());
	return out;
}

// --- --- --- --- --- Slots for connector highlight --- --- --- --- ---
void DDemandSinkCellPanel::mrgTableSelectionChanged() {
	for (auto& connectors : this->ref_fromConnectors)
		connectors.second->setSelected(false);

	for (auto& row : this->ui.MergeRatioInput_tableWidget->selectionModel()->selectedRows()) {
		int fromCellId = ((QSpinBox*)(this->ui.MergeRatioInput_tableWidget->cellWidget(row.row(), 0)))->value();
		if (this->ref_fromConnectors.count(fromCellId))
			this->ref_fromConnectors.at(fromCellId)->setSelected(true);
	}
}

void DDemandSinkCellPanel::divTableSelectionChanged() {
	for (auto& connectors : this->ref_toConnectors)
		connectors.second->setSelected(false);

	for (auto& row : this->ui.DivergeRatioInput_tableWidget->selectionModel()->selectedRows()) {
		int toCellId = ((QSpinBox*)(this->ui.DivergeRatioInput_tableWidget->cellWidget(row.row(), 0)))->value();
		if (this->ref_toConnectors.count(toCellId))
			this->ref_toConnectors.at(toCellId)->setSelected(true);
	}
}

// --- --- --- --- --- Customized Slots --- --- --- --- ---
void DDemandSinkCellPanel::enableConfirmButton() {
	this->ok_Buttons->ref_confirmButton->setEnabled(true);
}

void DDemandSinkCellPanel::setVisibleAddRemoveButtons(bool enable) {
	this->DemandInput_AddRemoveButtons->AddButton->setVisible(enable);
	this->DemandInput_AddRemoveButtons->RemoveButton->setVisible(enable);
}
