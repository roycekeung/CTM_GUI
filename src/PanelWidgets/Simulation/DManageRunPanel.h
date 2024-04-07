#pragma once

#include <QWidget>
#include <QDialog>
#include "ui_DManageRunPanel.h"

class DManageRunFunction;

class DManageRunPanel : public QDialog {
	Q_OBJECT

private:
	
	// --- --- --- --- --- UI Items --- --- --- --- ---

	Ui::DManageRunPanel ui;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DManageRunPanel();

	~DManageRunPanel();

	// --- --- --- --- --- Getters --- --- --- --- ---

	int getCurrentIndex();

	// --- --- --- --- --- Settings for Functions --- --- --- --- ---

	void setupButtons(DManageRunFunction* function);

	void clearTable();

	void addRow(QString&& name, QString&& details, QString&& sigSet, QString&& dmdSet, 
		QString&& time, double tolDmd, double tolDly, double avgDly);

};
