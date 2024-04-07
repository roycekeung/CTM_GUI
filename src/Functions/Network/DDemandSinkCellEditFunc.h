#pragma once

//Qt lib
#include <QtCore/QObject>

//DISCO GUI lib
#include "../I_Function.h"

// pre def
class DDemandSinkCellItem;
class DDemandSinkCellPanel;

class MouseWheelWidgetAdjustmentGuard;

class DDemandSinkCellEditFunc : public QObject, public I_Function {
	Q_OBJECT

private:
	MouseWheelWidgetAdjustmentGuard* noMoreScrolling = nullptr;

	// --- --- --- --- --- UI pannel and graphicitem--- --- --- --- ---
	DDemandSinkCellPanel* m_panelWidet = nullptr;
	DDemandSinkCellItem* m_CellItem = nullptr;
	int tmp_CellID;

	// inner data storing for table
	std::unordered_map<int, std::vector<std::tuple<int64_t, int64_t, double>>> tmp_model;
	int previous_dmdsetID = -1;

	// --- --- --- --- --- internal Functions --- --- --- --- ---
	void backupTableModel();
	void showDemandInputTable();
	void saveAllDemandLoadingintoCore_perSet(int DemSetID);

public:
	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---
	DDemandSinkCellEditFunc(int cellId);
	~DDemandSinkCellEditFunc();

	// --- --- --- --- --- override initFunctionHandler from I_Function --- --- --- --- ---
	void initFunctionHandler() override;


signals:
	void SendVisibleSignalToAddRemoveButton(bool enable);


public slots:
	// --- --- --- --- --- update and Edit Functions --- --- --- --- ---
	void addEmptyRow();
	void removerow();
	void updateDemandInputTablefromCore(const QString& dmd_setID);
	void checkAndEdit();
	void cancel();

	// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---
	bool keyPressEvent(QKeyEvent* keyEvent) override;
	bool mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

};

