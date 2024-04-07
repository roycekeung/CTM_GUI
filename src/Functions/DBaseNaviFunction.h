#pragma once

#include "I_Function.h"

#include <unordered_set>

#include <QObject>

class DJunctionModeFunction;
class QGraphicsItem;

/**
 An always running base funciton to aid basic navigation behaviour for the following items:

 Arc:
	Double click opens view / edit panel
	Selected will show all connectors for cells within
	Delete key delete

 Cell:
	Double click opens view / edit panel
	Selected will show all connectors

DmdSnkCell:
	Double click opens view / edit panel
	Selected will show all connectors
	Dragging
	Delete key delete

 Jct:
	Double click enters jct view
	Selected will show all connectors
	Dragging
	Delete key delete

 This function eliminates several functions as,
  view panels are create and forget;
  and select and show connectors behaves differently in jct view.

 Author: JLo
*/
class DBaseNaviFunction : public QObject, public I_Function {
	Q_OBJECT

private:
	
	bool m_ignoreEvents = false;
	bool m_netEditable = false;
	bool m_deleteMode = false;

	bool m_showAllDmdSnkCells = true;
	bool m_showAllCells = true;
	bool m_showAllArcs = true;
	bool m_showAllJcts = true;

	DJunctionModeFunction* t_jctModeFunction = nullptr;

	bool flag_selectionChanged = false;
	
public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DBaseNaviFunction();

	~DBaseNaviFunction();

	void initFunctionHandler() override;

	// --- --- --- --- --- State Getters --- --- --- --- ---

	bool getIsIgnoreEvents();
	bool getIsNetEditMode();
	bool getIsDeleteMode();
	bool getIsShowAllDmdSnkCells();
	bool getIsShowAllCells();
	bool getIsShowAllArcs();
	bool getIsShowAllJcts();

	// --- --- --- --- --- Getters --- --- --- --- ---

	DJunctionModeFunction* getJctModeFunction();

public slots:

	// --- --- --- --- --- State Changes --- --- --- --- ---

	void setIgnoreEvents(bool ignoresEvents);
	void setNetEditMode(bool isNetEditable);
	void setDeleteMode(bool isDeleting);

	void setShowAllDmdSnkCells(bool showAllDmdSnkCells);
	void setShowAllCells(bool showAllCells);	//sets all cell visibility and removes cell connectors if need be
	void setShowAllArcs(bool showAllArcs);
	void setShowAllJcts(bool showAllJcts);

	void initJctModeFunction(int jctId);

	// --- --- --- --- --- Display Util --- --- --- --- ---

	void sceneSelectionChanged();
	void showConnectorsOfSelected();

private:

	// --- --- --- --- --- Private Util --- --- --- --- ---

	bool viewItem(QGraphicsItem* item);
	bool editItem(QGraphicsItem* item);
	bool deleteItem(QGraphicsItem* item);

public:

	// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---

	bool keyPressEvent(QKeyEvent* keyEvent) override;

	bool mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

	bool mouseDoubleClickEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

	bool mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

	bool mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

};
