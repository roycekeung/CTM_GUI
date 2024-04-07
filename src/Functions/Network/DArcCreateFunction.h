#pragma once

#include <list>

#include <QPointF>
#include <QObject>
class QGraphicsPathItem;

#include "../I_Function.h"

class DArcPanel;
class DCellConnectorItem;
class DArcSpinePointItem;

class DArcCreateFunction : public QObject, public I_Function {
	Q_OBJECT

private:

	DArcPanel* m_panelWidet = nullptr;
	
	QGraphicsPathItem* t_line = nullptr;
	DCellConnectorItem* t_psuedoArc = nullptr;

	DArcSpinePointItem* t_movingPt = nullptr;
	QPointF t_dragStartPosition;

	std::list<DArcSpinePointItem*> t_spinePts;

	enum ClickStage { firstClick, createDrag, createdFirstArc, editDrag, addSpine };
	int m_clickStage = firstClick;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DArcCreateFunction();

	~DArcCreateFunction();

	void initFunctionHandler() override;

public slots:

	// --- --- --- --- --- Checking and creation Functions --- --- --- --- ---

	void checkAndCreateArc();

	void cancel();

	void recalLength();

public:

	// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---

	bool keyPressEvent(QKeyEvent* keyEvent) override;

	bool mouseDoubleClickEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

	bool mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

	bool mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

	bool mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

private:

	// --- --- --- --- --- Private Utils --- --- --- --- ---

	void resetPath();

};
