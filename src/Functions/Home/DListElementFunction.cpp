#include "DListElementFunction.h"

//DISCO GUI stuff
#include "PanelWidgets/Home/DListElementPanel.h"
#include "DDockWidgetHandler.h"
#include "GraphicsItems/DGraphicsScene.h"
#include "GraphicsItems/DGraphicsView.h"
#include "Functions/DBaseNaviFunction.h"
#include "Utils/DDefaultParamData.h"
#include "DZoomToExtentFunction.h"
#include "DFunctionHandler.h"

//DISCO GUI Customized Grpahic Items
#include "../GraphicsItems/DPolyArcItem.h"
#include "../GraphicsItems/DCellItem.h"
#include "../GraphicsItems/DDemandSinkCellItem.h"
#include "../GraphicsItems/DJctItem.h"
#include "../GraphicsItems/DJctLinkItem.h"

//Qt lib
#include <QString>
#include <QGraphicsSceneMouseEvent>
#include <QPushButton>
#include <QKeyEvent>
#include <QObject>

//DISCO Core lib
#include "Cell_DemandSink.h"
#include "VisInfo.h"

// for search results highlight
const QColor DListElementFunction::m_SearchResult_color = QColor{ 255,255,102, 150 };

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DListElementFunction::DListElementFunction(): QObject(nullptr), I_Function() {}

DListElementFunction::~DListElementFunction() {}

void DListElementFunction::initFunctionHandler() {
	//setup the widget
	this->m_panelWidet = new DListElementPanel{};
	this->m_panelWidet->connectFunction(this);
	this->ref_dockWidgetHandler->addFloatDockWidget(this->m_panelWidet, "List All Elements");

	// setup and display to treewidget
	this->reloadTree();

	//interaction from graphicsscene items changed to the displayTree, rebuit the table 
	QObject::connect(this->ref_graphicsScene, &DGraphicsScene::itemsChanged, this, &DListElementFunction::contentChanged);
	//interaction from graphicsscene items selection changes to the table widget selection changes
	QObject::connect(this->ref_graphicsScene, &QGraphicsScene::selectionChanged, this, &DListElementFunction::sceneSelectionChanged);

	//widget to trigger a tree reload when in focus
	QObject::connect(this->m_panelWidet, &DListElementPanel::focusedIn, this, &DListElementFunction::widgetFocusedIn);

	// connect the DDockWidgetHandler customized signal from DDockWidget default cross button to close this fuction
	QObject::connect(this->ref_dockWidgetHandler, &DDockWidgetHandler::floatDockWidgetClosed, this, &DListElementFunction::checkCloseFunctionPtr);
}

// --- --- --- --- --- Slots for selected items further actions--- --- --- --- ---

void DListElementFunction::contentChanged() {
	this->m_contentChanged = true;
}

void DListElementFunction::sceneSelectionChanged() {
	this->m_sceneSelectionChanged = true;
}

void DListElementFunction::treeSelectionChanged() {
	this->m_treeSelectionChanged = true;
}

void DListElementFunction::widgetFocusedIn() {
	if (this->m_contentChanged) {
		this->reloadTree();
		this->m_contentChanged = false;
	}
}

void DListElementFunction::widgetClickedSomething() {
	if (this->m_treeSelectionChanged) {
		this->highlightSelectedItemsOnScene();
		this->m_treeSelectionChanged = false;
	}
}

void DListElementFunction::zoomInScene() {
	// if junction mode or playback mode turn on is then ignored the following double click func
	if (!this->ref_baseNaviFunction->getIsIgnoreEvents()) {
		this->ref_functionHandler->addFunction(new DZoomToExtentFunction{});
	}
}

void DListElementFunction::findTextInTree(const QString& text) {

	this->resetSearchResults();

	if (text.size()) {
		for (int col = 0; col < this->m_panelWidet->getTreeWidget()->columnCount(); col++) {
			// set background state color to be pale yellow for matches
			for (QTreeWidgetItem* item : this->m_panelWidet->getTreeWidget()->findItems(
					text, Qt::MatchFlag::MatchRegularExpression | Qt::MatchFlag::MatchRecursive, col)) {
				// skip the topLevel tab items, do not search thru them
				if (item->parent() != nullptr) {
					this->m_searchResults.insert(item);

					item->setBackgroundColor(col, this->m_SearchResult_color);

					// automatically expand the tabs whereas the corresponding matches items located
					if (!item->parent()->isExpanded()) {
						item->parent()->setExpanded(true);
					}

				}
			}
		}

		// --- --- --- define current cur_SeachIndex --- --- --- 
		if (this->m_searchResults.size()) {
			this->cur_SeachIndex = 0;

			// locate and highlight the next search result
			auto itr = this->m_searchResults.begin();
			std::advance(itr, this->cur_SeachIndex);
			QTreeWidgetItem* item = *itr;
			item->setSelected(true);
			this->m_panelWidet->getTreeWidget()->scrollToItem(item);
		}
	}

	updateSearchResultsDisplay();
}

void DListElementFunction::searchUpwards() {
	if (this->m_searchResults.size()) {
		// clear all other selection
		this->m_panelWidet->getTreeWidget()->clearSelection();
		//update the cur index
		this->cur_SeachIndex++;
		if (this->cur_SeachIndex >= this->m_searchResults.size())
			this->cur_SeachIndex = 0;

		// locate and highlight the next search result
		auto itr = this->m_searchResults.begin();
		std::advance(itr, this->cur_SeachIndex);
		QTreeWidgetItem* item = *itr;
		item->setSelected(true);
		this->m_panelWidet->getTreeWidget()->scrollToItem(item);

		updateSearchResultsDisplay();
	}
}

void DListElementFunction::searchDownwards(){
	if (this->m_searchResults.size()) {
		// clear all other selection
		this->m_panelWidet->getTreeWidget()->clearSelection();
		//update the cur index
		this->cur_SeachIndex--;
		if (this->cur_SeachIndex < 0)
			this->cur_SeachIndex = this->m_searchResults.size() - 1;

		// locate and highlight the next search result
		auto itr = this->m_searchResults.begin();
		std::advance(itr, this->cur_SeachIndex);
		QTreeWidgetItem* item = *itr;
		item->setSelected(true);
		this->m_panelWidet->getTreeWidget()->scrollToItem(item);

		this->updateSearchResultsDisplay();
	}
}

// connect to the custom close button
void DListElementFunction::cancel() {
	//remove the demand sink cell panel
	this->ref_dockWidgetHandler->removeAndDeleteFloatDockWidget(this->m_panelWidet);

}

// receive signal from DDockWidgetHandler once the dockwidget is closed
void DListElementFunction::checkCloseFunctionPtr(QWidget* ptr) {
	if (ptr == this->m_panelWidet) {
		// end this function
		this->closeFunction();
	}
}

// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---

bool DListElementFunction::mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	if (this->m_sceneSelectionChanged) {
		this->highlightSelectedItemsInTree();
		this->m_sceneSelectionChanged = false;
	}
	return false;
}

// --- --- --- --- --- Private Utils --- --- --- --- ---

void DListElementFunction::resetSearchResults() {
	// --- --- --- clearing the previous search results background color --- --- --- 
	for (auto item : this->m_searchResults)
		for (int col = 0; col < this->m_panelWidet->getTreeWidget()->columnCount(); col++)
			item->setBackground(col, QBrush());

	// --- --- --- clear history and label display no matter what , before getting in the new search --- --- --- 
	this->m_searchResults.clear();
	this->cur_SeachIndex = -1;
}

// show the search results of how many matches in total and the current shown index
void DListElementFunction::updateSearchResultsDisplay() {
	if (this->m_searchResults.empty() && this->m_panelWidet->getFindInputlineEdit()->text().isEmpty()) {
		this->m_panelWidet->setFindResultlabelText(QString(""));
	}
	else {
		this->m_panelWidet->setFindResultlabelText(
			QString(QVariant(this->cur_SeachIndex + 1).toString())
			+ QString(" / ")
			+ QString(QVariant(this->m_searchResults.size()).toString()));
	}
}

void DListElementFunction::reloadTree() {
	// read data from core and display to treewidget

	// Clears all children of tree widget items except the top level
	for (auto i : this->m_panelWidet->getArcTopTreeItem()->takeChildren())
		delete i;
	for (auto i : this->m_panelWidet->getCellTopTreeItem()->takeChildren())
		delete i;
	for (auto i : this->m_panelWidet->getDmdSnkCellTopTreeItem()->takeChildren())
		delete i;
	for (auto i : this->m_panelWidet->getJctTopTreeItem()->takeChildren())
		delete i;
	for (auto i : this->m_panelWidet->getJctLnkTopTreeItem()->takeChildren())
		delete i;

	this->m_Arcs_child.clear();
	this->m_Cells_child.clear();
	this->m_DmdSnkCells_child.clear();
	this->m_Jcts_child.clear();
	this->m_JctLnks_child.clear();

	// --- --- --- --- --- --- --- --- --- --- Arcs --- --- --- --- --- --- --- --- --- ---

	// get total number of arcs that have been created and in usage confirmed by core
	auto& ref_ArcIds_InDGraphScn = this->ref_graphicsScene->getRefArcIds();
	auto* ref_arcTreeTop = m_panelWidet->getArcTopTreeItem();
	// set details for each subitems
	for (auto ArcId : ref_ArcIds_InDGraphScn) {
		//check existence
		if (this->ref_scn->get()->getNetwork().getArcIds().count(ArcId)) {
			//build child entry
			QTreeWidgetItem* child = new QTreeWidgetItem(ref_arcTreeTop);
			child->setText(0, QString::fromStdString(this->ref_scn->get()->getNetwork().getArc(ArcId)->getVisInfo().getName()));  // Item name
			child->setText(1, QString("%1").arg(this->ref_scn->get()->getNetwork().getArc(ArcId)->getId()));  // ID 
			child->setText(2, QString("%1").arg(this->ref_scn->get()->getNetwork().getArc(ArcId)->getLength()));  // Length 
			child->setText(3, QString("%1").arg(this->ref_scn->get()->getNetwork().getArc(ArcId)->getNumOfLanes()));  // Number of lane 
			child->setText(4, QString("%1").arg(this->ref_scn->get()->getNetwork().getArc(ArcId)->getVf()));  // Free flow speed 
			child->setText(5, QString("%1").arg(this->ref_scn->get()->getNetwork().getArc(ArcId)->getW()));  // Backward shockwave speed
			child->setText(6, QString("%1").arg(this->ref_scn->get()->getNetwork().getArc(ArcId)->getK()));  // Jam density
			child->setText(7, QString("%1").arg(this->ref_scn->get()->getNetwork().getArc(ArcId)->getQ()));  // Saturation Flow 

			//insert and record
			ref_arcTreeTop->insertChild(0, child);
			this->m_Arcs_child.emplace(ArcId, child);
		}
	}
	//show inserted count
	ref_arcTreeTop->setText(0, QString("Arcs (%1)").arg(ref_arcTreeTop->childCount()));


	// --- --- --- --- --- --- --- --- --- --- Cells --- --- --- --- --- --- --- --- --- --- 

	// get total number of Cells that have been created and in usage confirmed by core
	auto& ref_CellIds_InDGraphScn = this->ref_graphicsScene->getRefCellIds();
	auto* ref_cellTreeTop = m_panelWidet->getCellTopTreeItem();
	// set details for each subitems
	for (auto CellId : ref_CellIds_InDGraphScn) {
		//check existence
		if (this->ref_scn->get()->getNetwork().getCellIds().count(CellId)) {
			//build child entry
			QTreeWidgetItem* child = new QTreeWidgetItem(ref_cellTreeTop);
			child->setText(0, QString::fromStdString(this->ref_scn->get()->getNetwork().getCell(CellId)->getVisInfo().getName()));  // Item name
			child->setText(1, QString("%1").arg(this->ref_scn->get()->getNetwork().getCell(CellId)->getId()));  // ID 
			//child->setText(2, QString("%1").arg(this->ref_scn->get()->getNetwork().getCell(CellId)->getLength()));  // dont have Length getter 
			child->setText(3, QString("%1").arg(this->ref_scn->get()->getNetwork().getCell(CellId)->getNumOfLanes()));  // Number of lane 
			child->setText(4, QString("%1").arg(this->ref_scn->get()->getNetwork().getCell(CellId)->getFreeSpeed()));  // Free flow speed 
			child->setText(5, QString("%1").arg(this->ref_scn->get()->getNetwork().getCell(CellId)->getBackwardsWaveSpeed()));  // Backward shockwave speed
			child->setText(6, QString("%1").arg(this->ref_scn->get()->getNetwork().getCell(CellId)->getJamDensity()));  // Jam density
			child->setText(7, QString("%1").arg(this->ref_scn->get()->getNetwork().getCell(CellId)->getFlowCapacity()));  // Saturation Flow 

			//insert and record
			ref_cellTreeTop->insertChild(0, child);
			this->m_Cells_child.emplace(CellId, child);
		}
	}
	//show inserted count
	ref_cellTreeTop->setText(0, QString("Cells (%1)").arg(ref_cellTreeTop->childCount()));


	// --- --- --- --- --- --- --- --- --- --- Demand Sink Cells --- --- --- --- --- --- --- --- --- --- 

	// get total number of Demand Sink Cells that have been created and in usage confirmed by core
	auto& ref_DmdSnkCellIds_InDGraphScn = this->ref_graphicsScene->getRefDmdsnkCellIds();
	auto* ref_dmdCellTreeTop = m_panelWidet->getDmdSnkCellTopTreeItem();
	// set details for each subitems
	for (auto CellId : ref_DmdSnkCellIds_InDGraphScn) {
		//check existence
		if (this->ref_scn->get()->getNetwork().getDemandSinkCellIds().count(CellId)) {
			//build child entry
			QTreeWidgetItem* child = new QTreeWidgetItem(ref_dmdCellTreeTop);
			child->setText(0, QString::fromStdString(this->ref_scn->get()->getNetwork().getCell_DemandSink(CellId)->getVisInfo().getName()));  // Item name
			child->setText(1, QString("%1").arg(this->ref_scn->get()->getNetwork().getCell_DemandSink(CellId)->getId()));  // ID 
			//child->setText(2, QString("%1").arg(this->ref_scn->get()->getNetwork().getCell_DemandSink(CellId)->getLength()));  // dont have Length getter 
			child->setText(3, QString("%1").arg(this->ref_scn->get()->getNetwork().getCell_DemandSink(CellId)->getNumOfLanes()));  // Number of lane 
			child->setText(4, QString("%1").arg(this->ref_scn->get()->getNetwork().getCell_DemandSink(CellId)->getFreeSpeed()));  // Free flow speed 
			child->setText(5, QString("%1").arg(this->ref_scn->get()->getNetwork().getCell_DemandSink(CellId)->getBackwardsWaveSpeed()));  // Backward shockwave speed
			child->setText(6, QString("%1").arg(this->ref_scn->get()->getNetwork().getCell_DemandSink(CellId)->getJamDensity()));  // Jam density
			child->setText(7, QString("%1").arg(this->ref_scn->get()->getNetwork().getCell_DemandSink(CellId)->getFlowCapacity()));  // Saturation Flow 
			child->setText(8, this->ref_scn->get()->getNetwork().getCell_DemandSink(CellId)->getFlowInCapacity() == 0 ? QString("Demand") : QString("Sink"));   // Demand/Sink

			//insert and record
			ref_dmdCellTreeTop->insertChild(0, child);
			this->m_DmdSnkCells_child.emplace(CellId, child);
		}
	}
	//show inserted count
	ref_dmdCellTreeTop->setText(0, QString("Demand Sink Cells (%1)").arg(ref_dmdCellTreeTop->childCount()));


	// --- --- --- --- --- --- --- --- --- --- Junction nodes --- --- --- --- --- --- --- --- --- --- 
	// get total number of Junction nodes that have been created and in usage confirmed by core
	auto& ref_JctIds_InDGraphScn = this->ref_graphicsScene->getRefJctIds();
	auto* ref_jctTreeTop = m_panelWidet->getJctTopTreeItem();
	// set details for each subitems
	for (auto JctId : ref_JctIds_InDGraphScn) {
		//check existence
		if (this->ref_scn->get()->getNetwork().getJctIds().count(JctId)) {
			//build child entry
			QTreeWidgetItem* child = new QTreeWidgetItem(ref_jctTreeTop);
			child->setText(0, QString::fromStdString(this->ref_scn->get()->getNetwork().getJctNode(JctId).getVisInfo().getName()));  // Item name
			child->setText(1, QString("%1").arg(this->ref_scn->get()->getNetwork().getJctNode(JctId).getId()));  // ID 
			child->setText(9, this->ref_scn->get()->getNetwork().getJctNode(JctId).getHasSig() ? QString("Yes") : QString("No"));  // Signalized
			child->setText(10, this->ref_scn->get()->getNetwork().getJctNode(JctId).getHasYellowBox() ? QString("Yes") : QString("No"));  // Box Junction

			//insert and record
			ref_jctTreeTop->insertChild(0, child);
			this->m_Jcts_child.emplace(JctId, child);
		}
	}
	//show inserted count
	m_panelWidet->getJctTopTreeItem()->setText(0, QString("Junction nodes (%1)").arg(ref_jctTreeTop->childCount()));


	// --- --- --- --- --- --- --- --- --- --- Junction Links --- --- --- --- --- --- --- --- --- --- 
	//TODO probably need to change the widget ui to add jct node and jct link id into the thing

	//// get total number of Junction links that have been created and in usage confirmed by core
	//this->m_JctLnkIds_InDGraphScn = this->ref_graphicsScene->getRefJctLinkArcIds();
	//// total number of Junction Links arcs that have been created and in usage confirmed by core is accquired in arcs section
	//// set details for each subitems
	//for (auto ArcId : this->m_JctLnkIds_InDGraphScn) {
	//	QTreeWidgetItem* child = new QTreeWidgetItem(m_panelWidet->getJctLnkTopTreeItem());
	//	child->setText(0, QString::fromStdString(this->ref_scn->get()->getNetwork().getArc(ArcId)->getVisInfo().getName()));  // Item name
	//	child->setText(1, QString("%1").arg(this->ref_scn->get()->getNetwork().getArc(ArcId)->getId()));  // ID 
	//	child->setText(2, QString("%1").arg(this->ref_scn->get()->getNetwork().getArc(ArcId)->getLength()));  // Length 
	//	child->setText(3, QString("%1").arg(this->ref_scn->get()->getNetwork().getArc(ArcId)->getNumOfLanes()));  // Number of lane 
	//	child->setText(4, QString("%1").arg(this->ref_scn->get()->getNetwork().getArc(ArcId)->getVf()));  // Free flow speed 
	//	child->setText(5, QString("%1").arg(this->ref_scn->get()->getNetwork().getArc(ArcId)->getW()));  // Backward shockwave speed
	//	child->setText(6, QString("%1").arg(this->ref_scn->get()->getNetwork().getArc(ArcId)->getK()));  // Jam density
	//	child->setText(7, QString("%1").arg(this->ref_scn->get()->getNetwork().getArc(ArcId)->getQ()));  // Saturation Flow 

	//	// pushback to the local list 
	//	this->m_JctLnks_child.append(child);
	//}
	//// insert the child of Junction Links Arcs into corresponding topitem
	//m_panelWidet->getJctLnkTopTreeItem()->insertChildren(0, this->m_JctLnks_child);
	//m_panelWidet->getJctLnkTopTreeItem()->setText(0, QString("Junction Links (%1)").arg(this->m_JctLnkIds_InDGraphScn.size()));

	//// update the text searching results in every time changes on items chenages on treewidget
	//findTextInTree(this->m_panelWidet->getFindInputlineEdit()->text());

}

void DListElementFunction::highlightSelectedItemsOnScene() {
	// if junction mode or playback mode turn on is then ignored the following single click selction
	if (!this->ref_baseNaviFunction->getIsIgnoreEvents()) {

		// clear selection of each update of the selection signal emit from the panel treewidget table
		this->ref_graphicsScene->clearSelection();
		for (QTreeWidgetItem* selectedItem : this->m_panelWidet->getTreeWidget()->selectedItems()) {
			// get the ID of selected row
			int ID = selectedItem->text(1).toInt();
			// find the corresponding graphical items and set it to be selected
			if (selectedItem->parent() == this->m_panelWidet->getArcTopTreeItem()) {
				auto* ptr = this->ref_graphicsScene->getDArcItem(ID);
				if (ptr)
					ptr->setSelected(true);
			}
			else if (selectedItem->parent() == this->m_panelWidet->getCellTopTreeItem()) {
				auto* ptr = this->ref_graphicsScene->getDCellItem(ID);
				if (ptr)
					ptr->setSelected(true);
				// some cells might be set in the junction link whereas usually been hidden, 
				//	not been stored(not yet created unless in junction mode) in m_cell map
			}
			else if (selectedItem->parent() == this->m_panelWidet->getDmdSnkCellTopTreeItem()) {
				auto* ptr = this->ref_graphicsScene->getDmdSnkCellItem(ID);
				if (ptr)
					ptr->setSelected(true);
			}
			else if (selectedItem->parent() == this->m_panelWidet->getJctTopTreeItem()) {
				auto* ptr = this->ref_graphicsScene->getDJctItem(ID);
				if (ptr)
					ptr->setSelected(true);
			}
			else if (selectedItem->parent() == this->m_panelWidet->getJctLnkTopTreeItem()) {
				// problem : how to highlight Junction Links as they'd not yet been built
			}
		}
	}

	//reset the flag, such that these changes doesn't trigger another round of refresh
	this->m_sceneSelectionChanged = false;
}

void DListElementFunction::highlightSelectedItemsInTree() {
	// if junction mode or playback mode turn on is then ignored the following single click selction
	if (!this->ref_baseNaviFunction->getIsIgnoreEvents()) {

		//unselect everything
		this->m_panelWidet->getTreeWidget()->clearSelection();

		//jam everything into search as well
		this->resetSearchResults();
		
		bool first = true;
		//reselect based on whatever is found in graphics scene
		for (auto* item : this->ref_graphicsScene->selectedItems()) {
			QTreeWidgetItem* treeItem = nullptr;

			switch (item->type()) {
			case (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::ArcItem):
			{
				auto* ptr = dynamic_cast<DPolyArcItem*>(item);
				if (ptr) {
					int id = ptr->getArcId();
					if (this->m_Arcs_child.count(id))
						treeItem = this->m_Arcs_child.at(id);
				}
				break;
			}
			case (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::CellItem):
			{
				auto* ptr = dynamic_cast<DCellItem*>(item);
				if (ptr) {
					int id = ptr->getCellId();
					if (this->m_Cells_child.count(id)) 
						treeItem = this->m_Cells_child.at(id);
				}
				break;
			}
			case (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::DmdSnkCellItem):
			{
				auto* ptr = dynamic_cast<DDemandSinkCellItem*>(item);
				if (ptr) {
					int id = ptr->getCellId();
					if (this->m_DmdSnkCells_child.count(id)) 
						treeItem = this->m_DmdSnkCells_child.at(id);
				}
				break;
			}
			case (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::JctItem):
			{
				auto* ptr = dynamic_cast<DJctItem*>(item);
				if (ptr) {
					int id = ptr->getJctId();
					if (this->m_Jcts_child.count(id)) 
						treeItem = this->m_Jcts_child.at(id);
				}
				break;
			}
			default:
				break;
			}

			if (treeItem) {
				for (int col = 0; col < this->m_panelWidet->getTreeWidget()->columnCount(); col++)
					treeItem->setBackgroundColor(col, this->m_SearchResult_color);
				this->m_searchResults.insert(treeItem);
			}

			if (treeItem && first) {
				treeItem->setSelected(true);
				this->m_panelWidet->getTreeWidget()->scrollToItem(treeItem);
				first = false;
			}
		}

		if (this->m_searchResults.size())
			this->cur_SeachIndex = 0;

		this->updateSearchResultsDisplay();
	}

	//reset the flag, such that these changes doesn't trigger another round of refresh
	this->m_treeSelectionChanged = false;
}


// ************ ************ TODO in future ************ ************
// 
// 2. DDockWidgetHandler somewhere somehow should limit the number of this DListElementFunction created
// 
// 3. deletion synergy of any graphic items in graphicsscene, coz its keep reloading the network and the 
//		graphic items keep deleted and recreated
// 




