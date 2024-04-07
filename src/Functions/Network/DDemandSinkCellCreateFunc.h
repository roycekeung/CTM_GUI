#pragma once

//Qt lib
#include <QtCore/QObject>

//DISCO GUI lib
#include "../I_Function.h"

// pre def
class DDemandSinkCellItem;
class DDemandSinkCellPanel;

class DDemandSinkCellCreateFunc : public QObject, public I_Function{
	Q_OBJECT

private:
	// --- --- --- --- --- UI pannel and graphicitem--- --- --- --- ---
	DDemandSinkCellPanel* m_panelWidet = nullptr;
	DDemandSinkCellItem* m_CellItem = nullptr;

	// clicked behaviour
	enum ClickStage { NoClick, created };
	int m_clickStage = NoClick;

public:
	DDemandSinkCellCreateFunc();
	~DDemandSinkCellCreateFunc();

	// --- --- --- --- --- override initFunctionHandler from I_Function --- --- --- --- ---
	void initFunctionHandler() override;

public slots:
	// --- --- --- --- --- Checking and creation Functions --- --- --- --- ---
	void checkAndCreate();
	void cancel();

	// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---
	bool keyPressEvent(QKeyEvent* keyEvent) override;
	bool mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
};
