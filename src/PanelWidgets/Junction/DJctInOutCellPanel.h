#pragma once

#include <unordered_map>
//Qt stuff
#include <QWidget>
#include "ui_DJctInOutCellPanel.h"
//DISCO stuff
namespace DISCO2_API {
class Cell;
class Jct_Node;
}
//DISCO GUI stuff
class DOkCancelButtons;
class DCellConnectorItem;
class DPseudoJctLinkItem;
class DJctInOutCellEditFunction;

class DJctInOutCellPanel : public QWidget {
	Q_OBJECT

private:

	// --- --- --- --- --- Hint Texts --- --- --- --- ---

	const static std::string hintTextEditCell;
	const static std::string hintTextViewCell;

	// --- --- --- --- --- UI Items --- --- --- --- ---

	Ui::DJctInOutCellPanel ui;

	QLabel* hint_Label = nullptr;
	DOkCancelButtons* ok_Buttons = nullptr;

	// --- --- --- --- --- Ref to places --- --- --- --- ---

	const DISCO2_API::Cell* ref_cell;
	const DISCO2_API::Jct_Node& ref_jctNode;
	std::unordered_map<int, DPseudoJctLinkItem*> ref_jctLinks;

public:

	// --- --- --- --- --- Enum for Constructor --- --- --- --- ---

	enum Type_CellPanel { view, edit };

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DJctInOutCellPanel(Type_CellPanel type, const DISCO2_API::Cell* cell, const DISCO2_API::Jct_Node& jctNode,
		std::unordered_map<int, DPseudoJctLinkItem*>&& jctLinks);

	~DJctInOutCellPanel();

	// --- --- --- --- --- Settings for Functions --- --- --- --- ---

	void setEditable(bool editable = true);

	void enableConfirmButton();

	void setupEditButtons(DJctInOutCellEditFunction* function);

	void failMessage(QString&& msg);

	// --- --- --- --- --- Getters for relaying input form values --- --- --- --- ---

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
