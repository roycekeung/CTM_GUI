#pragma once

//std lib

//Qt lib
#include <QWidget>
#include <QObject>
#include <QPushButton>
#include <QTreeWidgetItem>
#include <QLineEdit>

//DISCO GUI lib
#include "ui_DListElementPanel.h"

// pre def
class DListElementFunction;

class DListElementPanel : public QWidget{
	Q_OBJECT
private:
	// --- --- --- --- --- Panel UI --- --- --- --- ---
	Ui::DListElementPanel ui;

public:
	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---
	DListElementPanel();
	~DListElementPanel();

	// --- --- --- --- --- Connection --- --- --- --- ---
	void connectFunction(DListElementFunction* function);

	// --- --- --- --- --- Setting --- --- --- --- ---
	void setFindResultlabelText(const QString& text);

	// --- --- --- --- --- getter --- --- --- --- ---
	QTreeWidgetItem* getArcTopTreeItem();
	QTreeWidgetItem* getCellTopTreeItem();
	QTreeWidgetItem* getDmdSnkCellTopTreeItem();
	QTreeWidgetItem* getJctTopTreeItem();
	QTreeWidgetItem* getJctLnkTopTreeItem();

	QTreeWidget* getTreeWidget();
	QLineEdit* getFindInputlineEdit();
	
signals:

	// --- --- --- --- --- Signal --- --- --- --- ---

	void focusedIn();

	void keyPressed();

public:

	// --- --- --- --- --- Re-implement functions for intercepting events  --- --- --- --- ---

	void focusInEvent(QFocusEvent* event) override;

	void keyReleaseEvent(QKeyEvent* event) override;

};
