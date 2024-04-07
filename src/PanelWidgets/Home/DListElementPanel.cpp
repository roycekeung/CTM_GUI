#include "DListElementPanel.h"

//DISCO GUI stuff
#include "../Functions/Home/DListElementFunction.h"
#include "../GraphicsItems/DGraphicsScene.h"
#include "../DBaseNaviFunction.h"

//Qt lib
#include <QString>
#include <QKeyEvent>
#include <QtWidgets/QLabel>

DListElementPanel::DListElementPanel(): QWidget(nullptr) {
	ui.setupUi(this);
	this->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
}

DListElementPanel::~DListElementPanel() {}

// --- --- --- --- --- Connection --- --- --- --- ---

void DListElementPanel::connectFunction(DListElementFunction* function) {
	// # All the buttons connections
	QObject::connect(this->ui.ZoomIn_pushButton, &QPushButton::clicked, function, &DListElementFunction::zoomInScene);
	QObject::connect(this->ui.SelectAll_pushButton, &QPushButton::clicked, this->ui.treeWidget, &QAbstractItemView::selectAll);
	QObject::connect(this->ui.UnselectAll_pushButton, &QPushButton::clicked, this->ui.treeWidget, &QAbstractItemView::clearSelection);
	QObject::connect(this->ui.ExpandAll_pushButton, &QPushButton::clicked, this->ui.treeWidget, &QTreeView::expandAll);
	QObject::connect(this->ui.CollapseAll_pushButton, &QPushButton::clicked, this->ui.treeWidget, &QTreeView::collapseAll);
	QObject::connect(this->ui.Close_pushButton, &QPushButton::clicked, function, &DListElementFunction::cancel);

	// # Selection Connections
	//change of selection on treetable then reflects onto graphical items on graphic scene
	QObject::connect(this->ui.treeWidget->selectionModel(), &QItemSelectionModel::selectionChanged, function, &DListElementFunction::treeSelectionChanged);
	QObject::connect(this->ui.treeWidget, &QTreeWidget::clicked, function, &DListElementFunction::widgetClickedSomething);
	QObject::connect(this->ui.treeWidget, &QTreeWidget::clicked, function, &DListElementFunction::widgetFocusedIn);
	QObject::connect(this, &DListElementPanel::keyPressed, function, &DListElementFunction::widgetClickedSomething);
	//double click on treetable item then zoom into the corresponding location
	QObject::connect(this->ui.treeWidget, &QTreeWidget::itemDoubleClicked, function, &DListElementFunction::zoomInScene);

	// # Search connections
	//change of selection on treetable then reflects onto graphical items on graphic scene
	QObject::connect(this->ui.FindInput_lineEdit, &QLineEdit::textChanged, function, &DListElementFunction::findTextInTree);
	//connect search buttons to function methods
	QObject::connect(this->ui.SearchUpward_pushButton, &QPushButton::clicked, function, &DListElementFunction::searchUpwards);
	QObject::connect(this->ui.SearchDownward_pushButton, &QPushButton::clicked, function, &DListElementFunction::searchDownwards);
}

// --- --- --- --- --- Setting --- --- --- --- ---

void DListElementPanel::setFindResultlabelText(const QString& text) {
	this->ui.FindResult_label->setText(text);
}

// --- --- --- --- --- getter --- --- --- --- ---
QTreeWidgetItem* DListElementPanel::getArcTopTreeItem() {
	return this->ui.treeWidget->topLevelItem(0);
}

QTreeWidgetItem* DListElementPanel::getCellTopTreeItem(){
	return this->ui.treeWidget->topLevelItem(1);
}

QTreeWidgetItem* DListElementPanel::getDmdSnkCellTopTreeItem() {
	return this->ui.treeWidget->topLevelItem(2);
}

QTreeWidgetItem* DListElementPanel::getJctTopTreeItem() {
	return this->ui.treeWidget->topLevelItem(3);
}

QTreeWidgetItem* DListElementPanel::getJctLnkTopTreeItem() {
	return this->ui.treeWidget->topLevelItem(4);
}

QTreeWidget* DListElementPanel::getTreeWidget() {
	return this->ui.treeWidget;
}

QLineEdit* DListElementPanel::getFindInputlineEdit() {
	return this->ui.FindInput_lineEdit;
}

// --- --- --- --- --- Re-implement functions for intercepting events  --- --- --- --- ---

void DListElementPanel::focusInEvent(QFocusEvent* event) {
	emit this->focusedIn();
	QWidget::focusInEvent(event);
}

void DListElementPanel::keyReleaseEvent(QKeyEvent* event) {
	if(event->key() == Qt::Key_Up || event->key() == Qt::Key::Key_Down)
		emit this->keyPressed();
	QWidget::keyReleaseEvent(event);
}
