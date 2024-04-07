#pragma once

#include <QPointF>
#include <QObject>

#include "../I_Function.h"

class DJctNodePanel;
class DJctItem;

class DJctCreateFunction : public QObject, public I_Function {
	Q_OBJECT

private:

	DJctNodePanel* m_panelWidet = nullptr;

	DJctItem* t_jctItem = nullptr;
	QPointF t_dragStartPosition;

	enum ClickStage { firstClick, createdFirstNode, editDrag };
	int m_clickStage = firstClick;

public:
	
	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DJctCreateFunction();

	~DJctCreateFunction();

	void initFunctionHandler() override;

public slots:

	// --- --- --- --- --- Checking and creation Functions --- --- --- --- ---

	void checkAndCreateJct();

	void cancel();

public:

	// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---

	bool keyPressEvent(QKeyEvent* keyEvent) override;

	bool mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

	bool mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

	bool mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
};
