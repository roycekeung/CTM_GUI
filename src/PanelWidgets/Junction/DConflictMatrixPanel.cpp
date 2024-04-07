#include "DConflictMatrixPanel.h"

#include "Functions/Junction/DConflictMatrixFunction.h"
#include "Utils/DConflictMatrixItem.h"

DConflictMatrixPanel::DConflictMatrixPanel(bool hasSig) : QDialog(nullptr),
        m_jLItemDelegate(new DConflictMatrixItem{}), m_sGItemDelegate(new DConflictMatrixItem{}),
        m_jLLegendItemDelegate(new DConflictMatrixItem{}), m_sGLegendItemDelegate(new DConflictMatrixItem{}) {

	ui.setupUi(this);
    this->ui.jLink_tableWidget->setItemDelegate(this->m_jLItemDelegate);
    this->ui.sigGp_tableWidget->setItemDelegate(this->m_sGItemDelegate);
    if(!hasSig)
        this->ui.tabWidget->setTabVisible(1, false);

    this->setupLengends();
 
    this->setWindowFlags(Qt::Window | Qt::WindowTitleHint);
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->ui.close_pushButton->setDefault(true);
    QObject::connect(this->ui.close_pushButton, &QPushButton::clicked, this, &QDialog::close);

    QObject::connect(
        this->ui.jLink_tableWidget->selectionModel(), &QItemSelectionModel::selectionChanged,
        this, &DConflictMatrixPanel::jLinkTableSelectionChanged); 
    QObject::connect(
        this->ui.sigGp_tableWidget->selectionModel(), &QItemSelectionModel::selectionChanged,
        this, &DConflictMatrixPanel::sigGpTableSelectionChanged);
}

DConflictMatrixPanel::~DConflictMatrixPanel() {
    delete this->m_jLItemDelegate;
    delete this->m_sGItemDelegate;
    delete this->m_jLLegendItemDelegate;
    delete this->m_sGLegendItemDelegate;
}

// --- --- --- --- --- Getters --- --- --- --- ---

QPushButton* DConflictMatrixPanel::getConflictButton() {
    return this->ui.conflictJL_pushButton;
}

QPushButton* DConflictMatrixPanel::getPriorityButton() {
    return this->ui.priority_pushButton;
}

QModelIndex DConflictMatrixPanel::getCurrentSelectedJLCell() {
	return this->ui.jLink_tableWidget->selectionModel()->currentIndex();
}

QModelIndex DConflictMatrixPanel::getCurrentSelectedSGCell() {
    return this->ui.sigGp_tableWidget->selectionModel()->currentIndex();
}

DConflictMatrixItem* DConflictMatrixPanel::getJLItemDelegate() {
    return this->m_jLItemDelegate;
}

DConflictMatrixItem* DConflictMatrixPanel::getSGItemDelegate() {
    return this->m_sGItemDelegate;
}

// --- --- --- --- --- Settings for Functions --- --- --- --- ---

void DConflictMatrixPanel::setupConnections(DConflictMatrixFunction* function) {
    QObject::connect(this->ui.close_pushButton, &QPushButton::clicked, function, &DConflictMatrixFunction::close);
    QObject::connect(this->ui.conflictJL_pushButton, &QPushButton::clicked, function, &DConflictMatrixFunction::clickedConflict);
    QObject::connect(this->ui.priority_pushButton, &QPushButton::clicked, function, &DConflictMatrixFunction::clickedPriority);
    QObject::connect(this->ui.reset_pushButton, &QPushButton::clicked, function, &DConflictMatrixFunction::clickedRevertAll);

    QObject::connect(this, &DConflictMatrixPanel::jLinkTableSelected, function, &DConflictMatrixFunction::hightlightJL);
    QObject::connect(this, &DConflictMatrixPanel::sigGpTableSelected, function, &DConflictMatrixFunction::hightlightSG);
}

void DConflictMatrixPanel::setJctLinkIds(std::vector<int>& ids) {
    this->m_jLItemDelegate->setSize(ids.size());
    this->setupTable(this->ui.jLink_tableWidget, ids);
}

void DConflictMatrixPanel::setSigGpIds(std::vector<int>& ids) {
    this->m_sGItemDelegate->setSize(ids.size());
    this->setupTable(this->ui.sigGp_tableWidget, ids);
}

void DConflictMatrixPanel::updateTables() {
    this->ui.jLink_tableWidget->viewport()->update();
    this->ui.sigGp_tableWidget->viewport()->update();
}

// --- --- --- --- --- Slots --- --- --- --- ---

void DConflictMatrixPanel::jLinkTableSelectionChanged() {
    auto index = this->getCurrentSelectedJLCell();

    //selection button
    this->ui.conflictJL_pushButton->setEnabled(index.row() != index.column());
    this->ui.priority_pushButton->setEnabled(
        this->m_jLItemDelegate->getType(index.row(), index.column()) != DConflictMatrixItem::ConflictType::noConflict
            && index.row() != index.column());

    //have function do the highlight
    emit this->jLinkTableSelected();
}

void DConflictMatrixPanel::sigGpTableSelectionChanged() {
    //have function do the highlight
    emit this->sigGpTableSelected();
}

// --- --- --- --- --- Private Utils --- --- --- --- ---

void DConflictMatrixPanel::setupTable(QTableWidget* table, std::vector<int>& ids) {
    //reset the table
    table->setColumnCount(0);
    table->setRowCount(0);
    table->setColumnCount(ids.size());
    table->setRowCount(ids.size());
    //put in the headers
    for (int i = 0; i < ids.size(); i++) {
        QTableWidgetItem* hItem = new QTableWidgetItem();
        hItem->setText(std::to_string(ids.at(i)).c_str());
        table->setHorizontalHeaderItem(i, hItem);

        QTableWidgetItem* vItem = new QTableWidgetItem();
        vItem->setText(std::to_string(ids.at(i)).c_str());
        table->setVerticalHeaderItem(i, vItem);
    }

    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
    table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
}

void DConflictMatrixPanel::setupLengends() {
    //Jct Link Table
    this->ui.jLinkLegend_tableWidget->setItemDelegate(this->m_jLLegendItemDelegate);
    this->ui.jLinkLegend_tableWidget->setColumnCount(2);
    this->ui.jLinkLegend_tableWidget->setRowCount(7);
    this->ui.jLinkLegend_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
    this->ui.jLinkLegend_tableWidget->horizontalHeader()->hide();
    this->ui.jLinkLegend_tableWidget->verticalHeader()->hide();

    this->m_jLLegendItemDelegate->setSize(7);
    this->m_jLLegendItemDelegate->setType(0, 0, DConflictMatrixItem::ConflictType::noConflict);
    this->m_jLLegendItemDelegate->setType(2, 0, DConflictMatrixItem::ConflictType::conflict);
    this->m_jLLegendItemDelegate->setType(4, 0, DConflictMatrixItem::ConflictType::priorityTop);
    this->m_jLLegendItemDelegate->setType(6, 0, DConflictMatrixItem::ConflictType::priorityLeft);

    this->ui.jLinkLegend_tableWidget->setItem(0, 1, 
        new QTableWidgetItem{ "There are no conflicts between the Junction Links" });
    this->ui.jLinkLegend_tableWidget->setItem(2, 1, 
        new QTableWidgetItem{ "The two Junction Links are conflicting.\r\nConflicting Junction Links have no effect in simulation." });    
    this->ui.jLinkLegend_tableWidget->setItem(4, 1, 
        new QTableWidgetItem{ "Priority is configured such that the Junction Link of the row will yield to the Junction Link of the column" });    
    this->ui.jLinkLegend_tableWidget->setItem(6, 1, 
        new QTableWidgetItem{ "Priority is configured such that the Junction Link of the column will yield to the Junction Link of the row" });

    this->ui.jLinkLegend_tableWidget->setWordWrap(true);
    this->ui.jLinkLegend_tableWidget->resizeRowsToContents();
    
    //Sig Gp Table
    this->ui.sigGpLegend_tableWidget->setItemDelegate(this->m_sGLegendItemDelegate);
    this->ui.sigGpLegend_tableWidget->setColumnCount(2);
    this->ui.sigGpLegend_tableWidget->setRowCount(5);
    this->ui.sigGpLegend_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
    this->ui.sigGpLegend_tableWidget->horizontalHeader()->hide();
    this->ui.sigGpLegend_tableWidget->verticalHeader()->hide();

    this->m_sGLegendItemDelegate->setSize(5);
    this->m_sGLegendItemDelegate->setType(0, 0, DConflictMatrixItem::ConflictType::noConflict);
    this->m_sGLegendItemDelegate->setType(2, 0, DConflictMatrixItem::ConflictType::conflict);
    this->m_sGLegendItemDelegate->setType(4, 0, DConflictMatrixItem::ConflictType::priorityConfigured);

    this->ui.sigGpLegend_tableWidget->setItem(0, 1,
        new QTableWidgetItem{ "There are no conflicts between the signal groups" });
    this->ui.sigGpLegend_tableWidget->setItem(2, 1,
        new QTableWidgetItem{ "The two signal groups are conflicting.\r\nConflicting signal groups have no effect in simulation." });
    this->ui.sigGpLegend_tableWidget->setItem(4, 1,
        new QTableWidgetItem{ "The two signal groups are conflicting.\r\nPriority between conflicting junction links are configured." });

    this->ui.sigGpLegend_tableWidget->setWordWrap(true);
    this->ui.sigGpLegend_tableWidget->resizeRowsToContents();
}
