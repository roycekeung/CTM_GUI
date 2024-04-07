#pragma once
//std lib
#include <unordered_map>

//Qt lib
#include <QWidget>
#include <QObject>

//DISCO GUI lib
#include "ui_DDemandSinkCellPanel.h"

//DISCO stuff
namespace DISCO2_API {
	class Cell_DemandSink;
}

// pre def
class DDemandSinkCellCreateFunc;
class DDemandSinkCellEditFunc;
class DDemandSinkCellViewFunc;

class DCellConnectorItem;
class DDemandSinkCellItem;

class DAddRemoveButtons;
class DOkCancelButtons;
struct DefaultParamDataSet;

class DDemandSinkCellPanel : public QWidget{
	Q_OBJECT
public:
	// --- --- --- --- --- Enum cell type --- --- --- --- ---
	enum CellType { Demand, Sink };
	enum Type_Panel { view, create, edit };

private:
	// --- --- --- --- --- Panel UI --- --- --- --- ---
	Ui::DDemandSinkCellPanel ui;
	DAddRemoveButtons* DemandInput_AddRemoveButtons = nullptr;
	DOkCancelButtons* ok_Buttons = nullptr;
	QLabel* hint_Label = nullptr;

	// --- --- --- --- --- storing of Enum type --- --- --- --- ---
	CellType typeOfCell = CellType::Demand;
	Type_Panel typeOfPanel;

	// --- --- --- --- --- Ref to places --- --- --- --- ---
	const DISCO2_API::Cell_DemandSink* ref_DmdSnkCell;
	std::unordered_map<int, DCellConnectorItem*> ref_toConnectors;
	std::unordered_map<int, DCellConnectorItem*> ref_fromConnectors;
	DefaultParamDataSet* ref_dParam = nullptr;

	// --- --- --- --- --- internal Functions --- --- --- --- ---
	void setEditable(bool editable = true);
	void switchPanelUIByCelltype(int CellType_index);
	void setTablesVisibleByCelltypeAndMode(Type_Panel typeOfPanel, CellType typeOfCell);

	// insert external button, text
	void insertHintLabel(Type_Panel typeOfPanel);
	void insertAddRemoveButtons(Type_Panel typeOfPanel);
	void insertOkButtons();

	// --- --- --- --- --- Private Utils --- --- --- --- ---
	void showCellIdAndDefaultParam();
	void showCellIdAndParam(bool editable);
	// for merge diverge table
	void addRowToTable(QTableWidget* table, int cellId, double ratio, bool isReadOnly);

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DDemandSinkCellPanel(Type_Panel typeOfPanel, DefaultParamDataSet* dparam,
		CellType typeOfCell = Demand, const DISCO2_API::Cell_DemandSink* DmdSnkCell = nullptr,
		std::unordered_map<int, DCellConnectorItem*>&& toConnectors = {},
		std::unordered_map<int, DCellConnectorItem*>&& fromConnectors = {});

	~DDemandSinkCellPanel();

	// --- --- --- --- --- Settings for Functions --- --- --- --- ---
	void connectCreateFunction(DDemandSinkCellCreateFunc* function);
	void connectEditFunction(DDemandSinkCellEditFunc* function);
	void connectViewFunction(DDemandSinkCellViewFunc* function);
	void connectItemToConfirmButton(DDemandSinkCellItem* Item);
	
	// --- --- --- --- --- show error tool --- --- --- --- --- 
	void failMessage(QString&& msg);

	// --- --- --- --- --- getter --- --- --- --- ---
	QString getNameInput();
	int getCellTypeInput();
	QString getDetailsInput();
	double getFlowCapInput();

	Ui::DDemandSinkCellPanel getUI();

	std::unordered_map<int, double> getMrgTableInput();
	std::unordered_map<int, double> getDivTableInput();

public slots:
	// --- --- --- --- --- Slots for connector highlight --- --- --- --- ---
	void mrgTableSelectionChanged();
	void divTableSelectionChanged();
	// --- --- --- --- --- Customized Slots --- --- --- --- ---
	void enableConfirmButton();
	void setVisibleAddRemoveButtons(bool enable);
};
