#pragma once

#include <vector>
#include <unordered_set>

#include <QObject>
#include <QGraphicsRectItem>

#include "Functions/I_Function.h"

class DSplitCellPanel;
class DCellItem;

class DSplitCellFunction : public QObject, public I_Function {
	Q_OBJECT

private:

	static const double selectionBoxBuffer;

	//Panel Widget
	DSplitCellPanel* m_panel = nullptr;

	//Core Rec Stuff
	bool m_hasSelectedRow = false;
	int m_selectedArcId = -1;
	int m_selectedRow = -1;
	int m_numOfLanes = -1;
	std::unordered_set<int> ref_orgCellIds;

	//Graphics Stuff
	QGraphicsRectItem* t_selectionBox;
	std::vector<DCellItem*> t_cells;
	std::vector<DCellItem*> ref_orgCells;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DSplitCellFunction();

	~DSplitCellFunction();

	void initFunctionHandler() override;

public slots:

	// --- --- --- --- --- Slots for Panel --- --- --- --- ---

	void cancel();

	void cancelSplit();

	void splitsInputChanged(std::vector<int> splits);

	void confirmSplits();

private:

	// --- --- --- --- --- Private Utils --- --- --- --- ---

	void cleanup();

	void redrawTempSplits(std::vector<int>& splits);

public:

	// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---

	bool keyPressEvent(QKeyEvent* keyEvent) override;

	bool mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

	bool mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

};
