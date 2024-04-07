#include "DSigGroupsPanel.h"

#include "Jct_Node.h"

#include <QSpinBox>
#include <QPushButton>

#include "Functions/Junction/DSigGroupsFunction.h"
#include "GraphicsItems/DPseudoJctLinkItem.h"
#include "Utils/MouseWheelWidgetAdjustmentGuard.h"

/**
 Extend the QPushButton so can store the sigGp the button is at
*/
class DEditJLinkButton : public QPushButton {
private:
	int m_sigGp;

public:
	DEditJLinkButton(QWidget* parent, int sigGpId) : QPushButton(parent), m_sigGp(sigGpId) {}
	
	virtual ~DEditJLinkButton() {}

	int getSigGpId() { return this->m_sigGp; }

};

// --- --- --- --- --- Hint Texts --- --- --- --- ---

const std::string DSigGroupsPanel::hintText{
"To create or delete a signal group use the add / remove buttons below the table\r\n\
\r\n\
To configure the junction links in a signal group,\r\n\
select the row in the table and click the edit button.\r\n\
Any existing junction links within the group will be highlighted\r\n\
Add/remove junction links to the group by clicking on the links\r\n\
\r\n"
};

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DSigGroupsPanel::DSigGroupsPanel(const DISCO2_API::Jct_Node& jctNode, std::unordered_map<int, DPseudoJctLinkItem*>&& jctLinks) 
		: QWidget(), ref_jctNode(jctNode), ref_jctLinks(jctLinks) {

	ui.setupUi(this);
	this->ui.hint_label->setWordWrap(true);
	this->ui.hint_label->setText(hintText.c_str());

	//setup the table
	this->ui.sigGp_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
	for (auto& sigGpId : this->ref_jctNode.getSigGpIds())
		this->addSigGpToTable(sigGpId);

	//connection
	QObject::connect(
		this->ui.sigGp_tableWidget->selectionModel(), &QItemSelectionModel::selectionChanged,
		this, &DSigGroupsPanel::sigGpTableSelectionChanged);
	QObject::connect(this->ui.remove_pushButton, &QPushButton::clicked, this, &DSigGroupsPanel::clickedDeleteSigGp);
}

DSigGroupsPanel::~DSigGroupsPanel() {

}

// --- --- --- --- --- Settings for Functions --- --- --- --- ---

void DSigGroupsPanel::setupConnections(DSigGroupsFunction* function) {
	QObject::connect(this->ui.add_pushButton, &QPushButton::clicked, function, &DSigGroupsFunction::addNewSigGp);
	QObject::connect(this->ui.close_pushButton, &QPushButton::clicked, function, &DSigGroupsFunction::cancel);
	QObject::connect(this, &DSigGroupsPanel::editSigGpF, function, &DSigGroupsFunction::editSigGp);
	QObject::connect(this, &DSigGroupsPanel::deleteSigGpF, function, &DSigGroupsFunction::deleteSigGp);
}

void DSigGroupsPanel::failMessage(QString&& msg) {
	this->ui.feedback_label->setText(msg);
	this->ui.feedback_label->update();
}

void DSigGroupsPanel::completedEditSigGp() {
	//enable all the buttons and change row button text
	for (int i = 0; i < this->ui.sigGp_tableWidget->rowCount(); ++i) {
		DEditJLinkButton* editButton = ((DEditJLinkButton*)(this->ui.sigGp_tableWidget->cellWidget(i, 2)));
		editButton->setEnabled(true);
		editButton->setText("Edit");
	}
	this->ui.add_pushButton->setEnabled(true);
	this->ui.remove_pushButton->setEnabled(true);
}

QTableWidget* DSigGroupsPanel::getTableWdiget() {

	return this->ui.sigGp_tableWidget;
}


// --- --- --- --- --- Slots --- --- --- --- ---

void DSigGroupsPanel::clickedEditSigGp() {
	DEditJLinkButton* button = dynamic_cast<DEditJLinkButton*>(sender());
	if (!button)
		return;
	int sigGpId = button->getSigGpId();

	//disable all the buttons and change row button text
	for (int i = 0; i < this->ui.sigGp_tableWidget->rowCount(); ++i) {
		if (((QSpinBox*)(this->ui.sigGp_tableWidget->cellWidget(i, 0)))->value() == sigGpId) {
			DEditJLinkButton* editButton = ((DEditJLinkButton*)(this->ui.sigGp_tableWidget->cellWidget(i, 2)));
			editButton->setText("Confirm");
		}
		else
			((DEditJLinkButton*)(this->ui.sigGp_tableWidget->cellWidget(i, 2)))->setEnabled(false);
	}
	this->ui.add_pushButton->setEnabled(false);
	this->ui.remove_pushButton->setEnabled(false);

	emit this->editSigGpF(sigGpId);
}

void DSigGroupsPanel::clickedDeleteSigGp() {
	for (auto& row : this->ui.sigGp_tableWidget->selectionModel()->selectedRows()) {
		int sigGpId = ((QSpinBox*)(this->ui.sigGp_tableWidget->cellWidget(row.row(), 0)))->value();

		emit this->deleteSigGpF(sigGpId);
	}
}

void DSigGroupsPanel::sigGpTableSelectionChanged() {
	//clear selection
	for (auto& connectors : this->ref_jctLinks)
		connectors.second->setSelected(false);

	//find selected rows (sigGpId)
	for (auto& row : this->ui.sigGp_tableWidget->selectionModel()->selectedRows()) {
		int sigGpId = ((QSpinBox*)(this->ui.sigGp_tableWidget->cellWidget(row.row(), 0)))->value();
		
		//select the jLinks
		if(this->ref_jctNode.getSigGpIds().count(sigGpId))
			for (auto& jLinkId : this->ref_jctNode.getSigGpJLinkIds(sigGpId))
				if(this->ref_jctLinks.count(jLinkId))
					this->ref_jctLinks.at(jLinkId)->setSelected(true);
	}
}

// --- --- --- --- --- Utils --- --- --- --- ---

void DSigGroupsPanel::addSigGpToTable(int sigGpId) {
	int row = this->ui.sigGp_tableWidget->rowCount();
	this->ui.sigGp_tableWidget->insertRow(row);

	// cellId set up
	QSpinBox* idCell = new QSpinBox(this->ui.sigGp_tableWidget);
	idCell->setMaximum(INT_MAX);
	idCell->setValue(sigGpId);
	idCell->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	idCell->setButtonSymbols(QAbstractSpinBox::NoButtons);
	this->ui.sigGp_tableWidget->setCellWidget(row, 0, idCell);
	idCell->setReadOnly(true);
	idCell->setFrame(false);  // no frame

	// jctLinkIds set up
	QLabel* jctLinkIdsLabel = new QLabel(this->ui.sigGp_tableWidget);
	jctLinkIdsLabel->setText("");
	jctLinkIdsLabel->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	this->ui.sigGp_tableWidget->setCellWidget(row, 1, jctLinkIdsLabel);

	//edit button
	DEditJLinkButton* editButton = new DEditJLinkButton(this->ui.sigGp_tableWidget, sigGpId);
	editButton->setText("Edit");
	this->ui.sigGp_tableWidget->setCellWidget(row, 2, editButton);
	QObject::connect(editButton, &DEditJLinkButton::clicked, this, &DSigGroupsPanel::clickedEditSigGp);

	// mingreen cell set up
	QSpinBox* minGreenCell = new QSpinBox(this->ui.sigGp_tableWidget);
	minGreenCell->setMaximum(1000);
	minGreenCell->setMinimum(0);
	if (this->ref_jctNode.getSigMinGreenMap().count(sigGpId))
		minGreenCell->setValue(this->ref_jctNode.getSigMinGreenMap().at(sigGpId));
	minGreenCell->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	minGreenCell->setButtonSymbols(QAbstractSpinBox::NoButtons);
	this->ui.sigGp_tableWidget->setCellWidget(row, 3, minGreenCell);
	minGreenCell->setFrame(false);  // no frame

	//put in JLinkIDs
	this->ui.sigGp_tableWidget->sortByColumn(0);
	this->updateSigGpTable(sigGpId);
}

void DSigGroupsPanel::updateSigGpTable(int sigGpId) {
	if (!this->ref_jctNode.getSigGpIds().count(sigGpId)) {
		this->removeSigGpFromTable(sigGpId);
		return;
	}

	for (int i = 0; i < this->ui.sigGp_tableWidget->rowCount(); ++i) {
		if (((QSpinBox*)(this->ui.sigGp_tableWidget->cellWidget(i, 0)))->value() == sigGpId) {
			QString tIds;
			auto& ids = this->ref_jctNode.getSigGpJLinkIds(sigGpId);
			for (auto& itr = ids.begin(); itr != ids.end(); ) {
				tIds.append(std::to_string(*itr).c_str());
				itr++;
				if(itr != ids.end())
					tIds.append(", ");
			}
			((QLabel*)(this->ui.sigGp_tableWidget->cellWidget(i, 1)))->setText(tIds);

		}
	}
}

void DSigGroupsPanel::removeSigGpFromTable(int sigGpId) {
	for (int i = 0; i < this->ui.sigGp_tableWidget->rowCount(); ++i) {
		if (((QSpinBox*)(this->ui.sigGp_tableWidget->cellWidget(i, 0)))->value() == sigGpId) {
			this->ui.sigGp_tableWidget->removeRow(i);
			break;
		}
	}
}
