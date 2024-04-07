#pragma once

//Qt lib
#include <QtCore/QObject>

//DISCO GUI lib
#include "../I_Function.h"

// pre def
class DDemandSinkCellItem;
class DDemandSinkCellPanel;


class DDemandSinkCellViewFunc : public QObject, public I_Function {
	Q_OBJECT

private:
	// --- --- --- --- --- UI pannel and Cell ID--- --- --- --- ---
	DDemandSinkCellPanel* m_panelWidet = nullptr;
	int tmp_CellID;

	// inner data storing for table
	std::unordered_map<int, std::vector<std::tuple<int, int, double>>> tmp_model;
	int previous_dmdsetID;

	// --- --- --- --- --- internal Functions --- --- --- --- ---
	void showDemandInputTable();

public:
	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---
	DDemandSinkCellViewFunc(int cellId);
	~DDemandSinkCellViewFunc();
	// --- --- --- --- --- override initFunctionHandler from I_Function --- --- --- --- ---
	void initFunctionHandler() override;

public slots:
	// --- --- --- --- --- update and show Functions --- --- --- --- ---
	void updateDemandInputTablefromCore(const QString& dmd_setID);
	void cancel();

	// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---
	bool mouseDoubleClickEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

};
