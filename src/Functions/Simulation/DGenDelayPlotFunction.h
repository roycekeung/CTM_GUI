#pragma once

#include <list>
#include <vector>

#include <QObject>
#include <QColor>

#include <I_Function.h>

namespace DISCO2_API {
	class Rec_All;
}

class DGenDelayPlotPanel;

class DGenDelayPlotFunction : public QObject, public I_Function {
	Q_OBJECT
private:

	// --- --- --- --- --- Colors --- --- --- --- ---

	const static QColor refColor_tSelectColor;
	const static QColor refColor_selectedColor;

	// --- --- --- --- --- Stuff --- --- --- --- ---

	const DISCO2_API::Rec_All* ref_rec;

	std::list<int> m_cellList;
	std::list<int> temp_cellList;
	int temp_lastEndingCell = -1;

	// --- --- --- --- --- UI Items --- --- --- --- ---
	DGenDelayPlotPanel* m_panel = nullptr;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DGenDelayPlotFunction(const DISCO2_API::Rec_All* rec);

	~DGenDelayPlotFunction();

	void initFunctionHandler() override;

public slots:

	// --- --- --- --- --- Slots for Panel --- --- --- --- ---

	void cancel();

	void removeLast();

	void removeAll();

	void confirmCellList();

private:

	// --- --- --- --- --- Private Utils --- --- --- --- ---

	void cleanupTemp();

	void showTempCells(int endCell, bool isThisCellOnly);

	void showNeededJctLinks(std::list<int>& seq);

	void addTempToList();

public:

	// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---

	bool keyPressEvent(QKeyEvent* keyEvent) override;

	bool keyReleaseEvent(QKeyEvent* keyEvent) override;

	bool mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

	bool mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

};
