#pragma once

#include <QPointF>
#include <QObject>

#include "../I_Function.h"

#include "VisInfo.h"

class DArcPanel;
class DCellConnectorItem;
class DArcSpinePointItem;

class DArcEditFunction : public QObject, public I_Function {
	Q_OBJECT

private:

	int m_arcId;

	DArcPanel* m_panelWidet = nullptr;

	DArcSpinePointItem* t_movingPt = nullptr;
	QPointF t_dragStartPosition;

	std::list<DArcSpinePointItem*> t_spinePts;

	DISCO2_API::VisInfo m_orgVisInfo;

	bool m_dragging = false;
	bool m_newSpinePt = false;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DArcEditFunction(int arcId);

	~DArcEditFunction();

	void initFunctionHandler() override;

public slots:

	// --- --- --- --- --- Checking and creation Functions --- --- --- --- ---

	void checkAndEditArc();

	void cleanup();

	void cancel();

	void recalLength();

public:

	// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---

	bool keyPressEvent(QKeyEvent* keyEvent) override;

	bool mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

	bool mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

	bool mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

private:

	// --- --- --- --- --- Private Utils --- --- --- --- ---

	void updateTempArc();

};
