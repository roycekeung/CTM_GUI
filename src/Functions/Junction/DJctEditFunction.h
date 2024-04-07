#pragma once

#include <QPointF>
#include <QObject>

#include "../I_Function.h"

class DJctNodePanel;
class DJctItem;

class DJctEditFunction : public QObject, public I_Function {
	Q_OBJECT

private:

	int m_jctId;

	DJctNodePanel* m_panelWidet = nullptr;
	
	DJctItem* ref_jctItem = nullptr;
	QPointF t_dragStartPosition;

	bool m_dragging = false;

public:
	
	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DJctEditFunction(int jctId);

	~DJctEditFunction();

	void initFunctionHandler() override;

public slots:

	// --- --- --- --- --- Checking and creation Functions --- --- --- --- ---

	void checkAndEditJct();

	void cancel();

	void cancelAndJctMode();	//quick and dirty JLo

public:

	// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---

	bool keyPressEvent(QKeyEvent* keyEvent) override;

	bool mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

	bool mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

	bool mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

};
