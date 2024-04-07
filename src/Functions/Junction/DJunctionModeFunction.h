#pragma once

#include "I_Function.h"

#include <unordered_map>

#include <QObject>
#include <QColor>

class DPseudoJctLinkItem;
class QGraphicsItem;

class DJunctionModeFunction : public QObject, public I_Function {
	Q_OBJECT

private:

	static const QColor refColor_inOutCell;

	int m_jctId;

	bool m_ignoreEvents = false;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DJunctionModeFunction(int jctId);

	~DJunctionModeFunction();

	void initFunctionHandler() override;

	// --- --- --- --- --- Getters --- --- --- --- ---

	int getCurrentJctId();

public slots:

	// --- --- --- --- --- Slots --- --- --- --- ---

	void rightDockWidgetVisChanged();

	void exitJctMode();

	void setIgnoreEvents(bool ignoresEvents);

	void fitJctInView();

private:

	// --- --- --- --- --- Private Utils --- --- --- --- ---

	void showViewPanel();

	int findJLinkId(QGraphicsItem* ptr);

	void highlightInOutCells();

public:

	// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---

	bool keyPressEvent(QKeyEvent* keyEvent) override;

	bool mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

	bool mouseDoubleClickEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

};
