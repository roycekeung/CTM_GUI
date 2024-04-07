#pragma once

#include <QObject>

#include "../I_Function.h"

class DArcPanel;
class DCellConnectorItem;
class DPseudoJctLinkItem;

class DJctLinkCreateFunction : public QObject, public I_Function {
	Q_OBJECT

private:

	DArcPanel* m_panelWidet = nullptr;

	DCellConnectorItem* t_dragStLine = nullptr;
	DPseudoJctLinkItem* t_pseudoJL = nullptr;

	int m_jctId = -1;
	int m_startCellId = -1;
	int m_endCellId = -1;

	enum ClickStage { firstClick, createDrag, createdFirstArc };
	int m_clickStage = firstClick;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DJctLinkCreateFunction();

	~DJctLinkCreateFunction();

	void initFunctionHandler() override;


public slots:

	// --- --- --- --- --- Checking and creation Functions --- --- --- --- ---

	void checkAndCreateJLink();

	void cancel();

	void recalLength();

public:

	// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---

	bool keyPressEvent(QKeyEvent* keyEvent) override;

	bool mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

	bool mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

};
