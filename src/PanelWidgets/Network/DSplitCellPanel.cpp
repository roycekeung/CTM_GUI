#include "DSplitCellPanel.h"

#include <unordered_set>

#include <QSpinBox>

#include "PanelWidgets/DAddRemoveButtons.h"
#include "PanelWidgets/DOkCancelButtons.h"
#include "Utils/MouseWheelWidgetAdjustmentGuard.h"
#include "Functions/Network/DSplitCellFunction.h"

// --- --- --- --- --- Hint Texts --- --- --- --- ---
std::string hintText{
"Click on any row of cells within an Arc to split the cells of that row\r\n\
The cell sequence are aligned from left to right\r\n\
You can add or remove splits using the buttons below the table\r\n\
You can modify the number of lanes each split represents using the up/down arrows on each row\r\n\
\r\n"
};

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DSplitCellPanel::DSplitCellPanel(QWidget *parent) : QWidget(parent) {
	ui.setupUi(this);

	this->ui.hint_label->setWordWrap(true);
	this->ui.hint_label->setText(hintText.c_str());

	// make buttons
	this->add_Buttons = new DAddRemoveButtons{ this };
	this->ui.verticalLayout_2->insertWidget(this->ui.verticalLayout_2->count(), this->add_Buttons);
	QObject::connect(this->add_Buttons->AddButton, &QPushButton::clicked, this, &DSplitCellPanel::addRow);
	QObject::connect(this->add_Buttons->RemoveButton, &QPushButton::clicked, this, &DSplitCellPanel::removeRows);

	this->ok_Buttons = new DOkCancelButtons{ this };
	this->ok_Buttons->ref_confirmButton->setText("Save Splits");
	this->ui.verticalLayout->insertWidget(this->ui.verticalLayout->count() - 1, this->ok_Buttons);
	this->ok_Buttons->ref_cancelButton->setDefault(true);
	QObject::connect(this->ok_Buttons->ref_cancelButton, &QPushButton::clicked, this, &DSplitCellPanel::clickedCancel);

	this->ui.cellSplit_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
}

DSplitCellPanel::~DSplitCellPanel() {
}

// --- --- --- --- --- Settings for Functions --- --- --- --- ---

void DSplitCellPanel::setShowing(bool isShow) {
	this->ui.groupBox->setVisible(isShow);
	this->ok_Buttons->ref_confirmButton->setVisible(isShow);
	this->ok_Buttons->ref_cancelButton->setText(isShow ? "Cancel" : "Exit Function");
}

void DSplitCellPanel::setParams(int arcId, int rowNum, int numOfLanes, const std::vector<int>& splits) {
	this->ui.arcId_label->setText(std::to_string(arcId).c_str());
	this->ui.rowNum_label->setText(std::to_string(rowNum).c_str());
	this->ui.laneNum_label->setText(std::to_string(numOfLanes).c_str());
	this->ref_numOfLane = numOfLanes;
	this->setSplits(splits);
}

void DSplitCellPanel::setupButtons(DSplitCellFunction* function) {
	QObject::connect(this, &DSplitCellPanel::splitsInputChanged, function, &DSplitCellFunction::splitsInputChanged);
	QObject::connect(this, &DSplitCellPanel::cancelSplit, function, &DSplitCellFunction::cancelSplit);
	QObject::connect(this, &DSplitCellPanel::cancel, function, &DSplitCellFunction::cancel);
	QObject::connect(this->ok_Buttons->ref_confirmButton, &QPushButton::clicked, function, &DSplitCellFunction::confirmSplits);
}

void DSplitCellPanel::setSplits(const std::vector<int>& splits) {
	this->ui.cellSplit_tableWidget->setRowCount(0);

	int i = 0;
	for (const int& split : splits) {
		this->addRowToTable(i, split);
		++i;
	}

	this->resetMaximum();
}

std::vector<int> DSplitCellPanel::getSplits() {
	std::vector<int> out{};
	for (int i = 0; i < this->ui.cellSplit_tableWidget->rowCount(); ++i) {
		int split = ((QSpinBox*)(this->ui.cellSplit_tableWidget->cellWidget(i, 1)))->value();
		if(split)
			out.push_back(split);
	}
	return out;
}

void DSplitCellPanel::failMessage(QString&& msg) {
	this->ok_Buttons->ref_feedbackLabel->setText(msg);
}

// --- --- --- --- --- Slots for Real Time Change --- --- --- --- ---

void DSplitCellPanel::splitsChanged() {
	//limit the input
	this->resetMaximum();

	//enable confirm iff satisfy requirement
	if (this->currentInputTotal() == this->ref_numOfLane) {
		this->ok_Buttons->ref_feedbackLabel->setText("");
		this->ok_Buttons->ref_confirmButton->setEnabled(true);
	}
	else {
		this->ok_Buttons->ref_feedbackLabel->setText("Number of lanes not used up");
		this->ok_Buttons->ref_confirmButton->setEnabled(false);
	}

	//tell function to update temp graphics
	emit this->splitsInputChanged(this->getSplits());
}

void DSplitCellPanel::addRow() {
	auto selectedRows = this->ui.cellSplit_tableWidget->selectionModel()->selectedRows();
	if (selectedRows.size()) {
		//insert at the last selected
		int insertAt = selectedRows.last().row();
		std::vector<int> currentSplits = this->getSplits();
		//clear and re-input everything
		this->ui.cellSplit_tableWidget->setRowCount(0);
		int j = 0;
		for (int i = 0; i < currentSplits.size(); ++i) {
			this->addRowToTable(j, currentSplits.at(i));
			++j;
			if (i == insertAt) {
				this->addRowToTable(j, 0);
				++j;
			}
		}
	}
	else {
		//just append last
		this->addRowToTable(this->ui.cellSplit_tableWidget->rowCount(), 0);
	}

	this->splitsChanged();
}

void DSplitCellPanel::removeRows() {
	//get all the things
	std::vector<int> currentSplits = this->getSplits();
	std::unordered_set<int> selected{};
	for (auto& row : this->ui.cellSplit_tableWidget->selectionModel()->selectedRows())
		selected.insert(row.row());

	//clear and re-input everything
	this->ui.cellSplit_tableWidget->setRowCount(0);
	int j = 0;
	for (int i = 0; i < currentSplits.size(); ++i)
		if (!selected.count(i)) {
			this->addRowToTable(j, currentSplits.at(i));
			++j;
		}

	this->splitsChanged();
}

void DSplitCellPanel::clickedCancel() {
	if (this->ui.groupBox->isVisible())
		emit this->cancelSplit();
	else
		emit this->cancel();
}

// --- --- --- --- --- Private Utils --- --- --- --- ---

void DSplitCellPanel::addRowToTable(int id, int laneCount) {
	int row = this->ui.cellSplit_tableWidget->rowCount();
	this->ui.cellSplit_tableWidget->insertRow(row);

	// pos id set up
	QLabel* idCell = new QLabel(this->ui.cellSplit_tableWidget);
	idCell->setText(std::to_string(id).c_str());
	idCell->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	this->ui.cellSplit_tableWidget->setCellWidget(row, 0, idCell);

	// lane rep set up
	QSpinBox* splitCell = new QSpinBox(this->ui.cellSplit_tableWidget);
	splitCell->setMinimum(0);
	splitCell->setValue(laneCount);
	splitCell->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	splitCell->setButtonSymbols(QAbstractSpinBox::PlusMinus);
	this->ui.cellSplit_tableWidget->setCellWidget(row, 1, splitCell);
	splitCell->setReadOnly(false);
	splitCell->setFrame(false);  // no frame
	splitCell->installEventFilter(new MouseWheelWidgetAdjustmentGuard(splitCell));

	QObject::connect(splitCell, QOverload<int>::of(&QSpinBox::valueChanged), this, &DSplitCellPanel::splitsChanged);
}

void DSplitCellPanel::resetMaximum() {
	int total = this->currentInputTotal();
	for (int i = 0; i < this->ui.cellSplit_tableWidget->rowCount(); ++i) {
		QSpinBox* splitCell = (QSpinBox*)(this->ui.cellSplit_tableWidget->cellWidget(i, 1));
		splitCell->setMaximum(this->ref_numOfLane - (total - splitCell->value()));
	}
}

int DSplitCellPanel::currentInputTotal() {
	int count = 0;
	for (int i = 0; i < this->ui.cellSplit_tableWidget->rowCount(); ++i)
		count += ((QSpinBox*)(this->ui.cellSplit_tableWidget->cellWidget(i, 1)))->value();
	return count;
}
