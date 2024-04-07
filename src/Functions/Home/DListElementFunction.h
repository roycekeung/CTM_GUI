#pragma once

//std lib
#include <unordered_set>
#include <map>

//Qt lib
#include <QtCore/QObject>
#include <QList>
#include <QTreeWidgetItem>
#include <QGraphicsItem>
#include <QColor>

//DISCO GUI lib
#include "../I_Function.h"

// pre def
class DListElementPanel;
class DDockWidget;

class DListElementFunction : public QObject, public I_Function {
	Q_OBJECT
private:

	const static QColor m_SearchResult_color;

	// --- --- --- --- --- UI pannel and graphicitem--- --- --- --- ---
	DListElementPanel* m_panelWidet = nullptr;

	// --- --- --- --- --- QList for SubItems--- --- --- --- ---
	std::unordered_map<int, QTreeWidgetItem*> m_Arcs_child{};
	std::unordered_map<int, QTreeWidgetItem*> m_Cells_child{};
	std::unordered_map<int, QTreeWidgetItem*> m_DmdSnkCells_child{};
	std::unordered_map<int, QTreeWidgetItem*> m_Jcts_child{};
	std::unordered_map<int, QTreeWidgetItem*> m_JctLnks_child{};	//TODO this will need to be changed

	// --- --- --- --- --- Search history records --- --- --- --- ---
	bool m_contentChanged = false;
	bool m_sceneSelectionChanged = false;
	bool m_treeSelectionChanged = false;
	
	std::unordered_set<QTreeWidgetItem*> m_searchResults;
	int cur_SeachIndex = -1;

public:
	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---
	
	DListElementFunction();
	
	~DListElementFunction();

	void initFunctionHandler() override;

public slots:

	// --- --- --- --- --- Slots for selected items further actions --- --- --- --- ---

	void contentChanged();
	void sceneSelectionChanged();
	void treeSelectionChanged();

	void widgetFocusedIn();
	void widgetClickedSomething();

	void zoomInScene();

	void findTextInTree(const QString& text);
	void searchUpwards();
	void searchDownwards();

	void cancel();

	// receive signal from DDockWidgetHandler once the dockwidget is closed
	void checkCloseFunctionPtr(QWidget* ptr);

public:

	// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---

	// confirm scene user input action that triggers a selection change
	bool mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

private:

	// --- --- --- --- --- Private Utils --- --- --- --- ---

	void resetSearchResults();

	// show the search results of how many matches in total and the current shown index
	void updateSearchResultsDisplay();

	void reloadTree();
	void highlightSelectedItemsOnScene();
	void highlightSelectedItemsInTree();

};
