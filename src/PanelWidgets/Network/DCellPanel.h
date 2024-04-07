#pragma once

#include <unordered_map>
//Qt stuff
#include <QWidget>
#include <QString>
#include <QtWidgets/QLabel>
#include "ui_DCellPanel.h"
//DISCO stuff
namespace DISCO2_API {
class Cell;
}
//DISCO GUI stuff
class DOkCancelButtons;
class DCellCreateFunction;
class DCellEditFunction;
class DCellConnectorItem;

class DCellPanel : public QWidget {
	Q_OBJECT

private:

	// --- --- --- --- --- UI Items --- --- --- --- ---

	Ui::DCellPanel ui;

	DOkCancelButtons* ok_Buttons = nullptr;

	// --- --- --- --- --- Ref to places --- --- --- --- ---

	const DISCO2_API::Cell* ref_cell;
	std::unordered_map<int, DCellConnectorItem*> ref_toConnectors;
	std::unordered_map<int, DCellConnectorItem*> ref_fromConnectors;

public:

	// --- --- --- --- --- Enum for Constructor --- --- --- --- ---

	enum Type_CellPanel { view, edit };

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DCellPanel(Type_CellPanel type, const DISCO2_API::Cell* cell, 
		std::unordered_map<int, DCellConnectorItem*>&& toConnectors, 
		std::unordered_map<int, DCellConnectorItem*>&& fromConnectors);

	~DCellPanel();

	// --- --- --- --- --- Settings for Functions --- --- --- --- ---

	void setEditable(bool editable = true);

	void enableConfirmButton();

	void setupEditButtons(DCellEditFunction* function);

	void failMessage(QString&& msg);

	// --- --- --- --- --- Getters for relaying input form values --- --- --- --- ---

	double getQInput();
	double getKInput();
	double getWInput();
	double getVfInput();
	QString getNameInput();
	QString getDetailsInput();
	std::unordered_map<int, double> getMrgTableInput();
	std::unordered_map<int, double> getDivTableInput();

public slots:

	// --- --- --- --- --- Slots for connector highlight --- --- --- --- ---

	void mrgTableSelectionChanged();

	void divTableSelectionChanged();

private:

	// --- --- --- --- --- Private Utils --- --- --- --- ---

	void insertHintLabel(Type_CellPanel type);

	void insertOkButtons();

	void showCellIdAndParam(bool editable);

	void addRowToTable(QTableWidget* table, int cellId, double ratio, bool isReadOnly);

};
