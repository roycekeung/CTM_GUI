#include "DBaseNaviFunction.h"

#include "Scenario.h"
#include "Net_Container.h"
#include "Editor_Net.h"

#include "GraphicsItems/DGraphicsScene.h"
#include "DDockWidgetHandler.h"

#include "DGraphicsScene.h"
#include "DGraphicsView.h"
#include "GraphicsItems/DPolyArcItem.h"
#include "GraphicsItems/DCellItem.h"
#include "GraphicsItems/DJctItem.h"
#include "GraphicsItems/DDemandSinkCellItem.h"

#include "PanelWidgets/Network/DArcPanel.h"
#include "PanelWidgets/Network/DCellPanel.h"
#include "PanelWidgets/Junction/DJctNodePanel.h"
#include "PanelWidgets/Network/DDemandSinkCellPanel.h"

#include "DFunctionHandler.h"
#include "Functions/Network/DArcEditFunction.h"
#include "Functions/Network/DCellEditFunction.h"
#include "Functions/Network/DDemandSinkCellEditFunc.h"
#include "Functions/Network/DDemandSinkCellViewFunc.h"
#include "Functions/Junction/DJunctionModeFunction.h"
#include "Functions/Junction/DJctEditFunction.h"

#include <QGraphicsItem>
#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DBaseNaviFunction::DBaseNaviFunction() : QObject(nullptr) {

}

DBaseNaviFunction::~DBaseNaviFunction() {

}

void DBaseNaviFunction::initFunctionHandler() {
	//make connections
	QObject::connect(this->ref_graphicsScene, &QGraphicsScene::selectionChanged, this, &DBaseNaviFunction::sceneSelectionChanged);
}

// --- --- --- --- --- State Getters --- --- --- --- ---

bool DBaseNaviFunction::getIsIgnoreEvents() {
	return this->m_ignoreEvents;
}

bool DBaseNaviFunction::getIsNetEditMode() {
	return this->m_netEditable;
}

bool DBaseNaviFunction::getIsDeleteMode() {
	return this->m_deleteMode;
}

bool DBaseNaviFunction::getIsShowAllDmdSnkCells() {
	return this->m_showAllDmdSnkCells;
}

bool DBaseNaviFunction::getIsShowAllCells() {
	return this->m_showAllCells;
}

bool DBaseNaviFunction::getIsShowAllArcs() {
	return this->m_showAllArcs;
}

bool DBaseNaviFunction::getIsShowAllJcts() {
	return this->m_showAllJcts;
}

// --- --- --- --- --- Getters --- --- --- --- ---

DJunctionModeFunction* DBaseNaviFunction::getJctModeFunction() {
	return this->t_jctModeFunction;
}

// --- --- --- --- --- State Changes --- --- --- --- ---

void DBaseNaviFunction::setIgnoreEvents(bool ignoresEvents) {
	this->m_ignoreEvents = ignoresEvents;
}

void DBaseNaviFunction::setNetEditMode(bool isNetEditable) {
	this->m_netEditable = isNetEditable;
}

void DBaseNaviFunction::setDeleteMode(bool isDeleting) {
	this->m_deleteMode = isDeleting;
	if(!isDeleting)
		this->ref_graphicsView->viewport()->setCursor(Qt::ArrowCursor);
}

void DBaseNaviFunction::setShowAllDmdSnkCells(bool showAllDmdSnkCells) {
	if (showAllDmdSnkCells != this->m_showAllDmdSnkCells) {
		//only invoke if there is a change
		this->ref_graphicsScene->setShowAllDmdSnkCell(showAllDmdSnkCells);
		// update the new state 
		this->m_showAllDmdSnkCells = showAllDmdSnkCells;
	}
}

void DBaseNaviFunction::setShowAllCells(bool showAllCells) {
	if (showAllCells != this->m_showAllCells) {
		//only invoke if there is a change
		this->ref_graphicsScene->setShowAllCell(showAllCells);
		// update the new state 
		this->m_showAllCells = showAllCells;

		if (!showAllCells) {
			//delete all the connectors cell to cell
			this->ref_graphicsScene->removeAllDCellConnector();
		}
	}
}

void DBaseNaviFunction::setShowAllArcs(bool showAllArcs) {
	if (showAllArcs != this->m_showAllArcs) {
		//only invoke if there is a change
		this->ref_graphicsScene->setShowAllArc(showAllArcs);
		// update the new state 
		this->m_showAllArcs = showAllArcs;
	}
}

void DBaseNaviFunction::setShowAllJcts(bool showAllJcts) {
	if (showAllJcts != this->m_showAllJcts) {
		//only invoke if there is a change
		this->ref_graphicsScene->setShowAllJct(showAllJcts);
		// update the new state 
		this->m_showAllJcts = showAllJcts;

		if (!showAllJcts) {
			//delete all the connectors cell to jct
			this->ref_graphicsScene->removeAllDCellConnector_C2J();
		}
	}
}

void DBaseNaviFunction::initJctModeFunction(int jctId) {
	this->t_jctModeFunction = new DJunctionModeFunction{ jctId };
	this->ref_functionHandler->addFunction(this->t_jctModeFunction);
}

// --- --- --- --- --- Display Util --- --- --- --- ---

void DBaseNaviFunction::sceneSelectionChanged() {
	//set flag so once mouse released to trigger the drawing of connectors
	this->flag_selectionChanged = true;
}

void DBaseNaviFunction::showConnectorsOfSelected() {
	if (this->m_ignoreEvents)
		return;

	//clean all
	this->ref_graphicsScene->removeAllDCellConnector();
	//spam create all
	this->ref_graphicsScene->addConnectorsOfSelectedItems();
}

// --- --- --- --- --- Private Util --- --- --- --- ---

bool DBaseNaviFunction::viewItem(QGraphicsItem* item) {
	switch (item->type()) {
	case (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::ArcItem):
	{
		DPolyArcItem* arcItem = dynamic_cast<DPolyArcItem*>(item);
		//simply addin the viewing panel
		this->ref_dockWidgetHandler->setRightDockWidget(
			new DArcPanel(DArcPanel::Type_ArcPanel::view, this->ref_scn->get()->getNetwork().getArc(arcItem->getArcId())),
			"View Arc", true);
		return true;
	}
	case (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::CellItem):
	{
		DCellItem* cellItem = dynamic_cast<DCellItem*>(item);
		//simply addin the viewing panel
		this->ref_dockWidgetHandler->setRightDockWidget(
			new DCellPanel(DCellPanel::Type_CellPanel::view, this->ref_scn->get()->getNetwork().getCell(cellItem->getCellId()),
				this->ref_graphicsScene->getToCellConnectors(cellItem->getCellId()),
				this->ref_graphicsScene->getFromCellConnectors(cellItem->getCellId())),
			"View Cell", true);
		return true;
	}
	case (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::JctItem):
	{
		DJctItem* jctItem = dynamic_cast<DJctItem*>(item);
		//simply addin the viewing panel
		this->ref_dockWidgetHandler->setRightDockWidget(
			new DJctNodePanel(DJctNodePanel::Type_JctPanel::view, &(this->ref_scn->get()->getNetwork().getJctNode(jctItem->getJctId()))),
			"View Junction", true);
		return true;
	}
	case (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::DmdSnkCellItem):
	{
		DDemandSinkCellItem* DmdSnkCellItem = dynamic_cast<DDemandSinkCellItem*>(item);
		//simply addin the viewing panel
		this->ref_functionHandler->addFunction(new DDemandSinkCellViewFunc{ DmdSnkCellItem->getCellId() });
		return true;
	}
	default:
		return false;
	}
}

bool DBaseNaviFunction::editItem(QGraphicsItem* item) {
	switch (item->type()) {
	case (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::ArcItem):
	{
		DPolyArcItem* arcItem = dynamic_cast<DPolyArcItem*>(item);
		//invoke the arc editing function
		this->ref_functionHandler->addFunction(new DArcEditFunction{ arcItem->getArcId() });
		return true;
	}
	case (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::CellItem):
	{
		DCellItem* cellItem = dynamic_cast<DCellItem*>(item);
		//invoke the cell editing function
		this->ref_functionHandler->addFunction(new DCellEditFunction{ cellItem->getCellId() });
		return true;
	}
	case (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::JctItem):
	{
		int jctId = (dynamic_cast<DJctItem*>(item))->getJctId();
		//invoke the Jct Edit function
		this->ref_functionHandler->addFunction(new DJctEditFunction{ jctId });
		return true;
	}
	case (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::DmdSnkCellItem):
	{
		DDemandSinkCellItem* DmdSnkCellItem = dynamic_cast<DDemandSinkCellItem*>(item);
		//invoke the demand sink cell editing function
		this->ref_functionHandler->addFunction(new DDemandSinkCellEditFunc{ DmdSnkCellItem->getCellId() });
		return true;
	}
	default:
		return false;
	}
}

bool DBaseNaviFunction::deleteItem(QGraphicsItem* item) {
	switch (item->type()) {
	case (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::JctItem):
	{
		DJctItem* jctItem = dynamic_cast<DJctItem*>(item);
		//check existence before delete
		if (this->ref_scn->get()->getNetwork().getJctIds().count(jctItem->getJctId())) {
			this->ref_scn->get()->getNetEditor().jct_deleteJunctNode(jctItem->getJctId());
			return true;
		}
		break;
	}
	case (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::ArcItem):
	{
		DPolyArcItem* arcItem = dynamic_cast<DPolyArcItem*>(item);
		//check existence before delete
		if (this->ref_scn->get()->getNetwork().getArcIds().count(arcItem->getArcId())) {
			this->ref_scn->get()->getNetEditor().arc_delete(arcItem->getArcId());
			return true;
		}
		break;
	}
	case (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::DmdSnkCellItem):
	{
		DDemandSinkCellItem* DmdSnkCellItem = dynamic_cast<DDemandSinkCellItem*>(item);
		//check existence before delete
		if (this->ref_scn->get()->getNetwork().getDemandSinkCellIds().count(DmdSnkCellItem->getCellId())) {
			int del_report = this->ref_scn->get()->getNetEditor().cell_delete(DmdSnkCellItem->getCellId());
			return true;
		}
		break;
	}
	}
	return false;
}

// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---

bool DBaseNaviFunction::keyPressEvent(QKeyEvent* keyEvent) {
	if (this->m_ignoreEvents || !this->m_netEditable || keyEvent->key() != Qt::Key::Key_Delete)
		return false;

	//delete everything selected from core
	for (auto& item : this->ref_graphicsScene->selectedItems()) {
		this->deleteItem(item);
	}
	this->ref_graphicsScene->reloadNet();

	return true;
}

bool DBaseNaviFunction::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	if (this->m_ignoreEvents || this->ref_graphicsScene->items(mouseEvent->scenePos()).isEmpty())
		return false;

	//grab the item that's clicked on
	auto* topItem = this->ref_graphicsScene->items(mouseEvent->scenePos()).first();
	
	if (mouseEvent->modifiers().testFlag(Qt::KeyboardModifier::ControlModifier)) {
		//multi-selection, no viewing stuff
		this->ref_dockWidgetHandler->closeRightDockWidget();
		return false;
	}
	else if (this->m_deleteMode) {
		this->deleteItem(topItem);
		this->ref_graphicsScene->reloadNet();
		mouseEvent->accept();
		return true;
	}
	else {
		this->ref_dockWidgetHandler->closeRightDockWidget();

		//force rebuild at correct time
		this->ref_graphicsScene->clearSelection();
		topItem->setSelected(true);
		if (topItem->type() != (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::CellConnectorItem))
			this->showConnectorsOfSelected();
		this->flag_selectionChanged = false;
		this->ref_graphicsScene->update();

		if (this->viewItem(topItem)) {
			mouseEvent->accept();
			return true;
		}
		return false;
	}
}

bool DBaseNaviFunction::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	if (this->m_ignoreEvents || this->ref_graphicsScene->items(mouseEvent->scenePos()).isEmpty())
		return false;
	
	//grab the item that's clicked on
	auto* topItem = this->ref_graphicsScene->items(mouseEvent->scenePos()).first();

	//force rebuild at correct time
	this->ref_graphicsScene->clearSelection();
	topItem->setSelected(true);
	if (topItem->type() != (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::CellConnectorItem))
		this->showConnectorsOfSelected();
	this->flag_selectionChanged = false;
	this->ref_graphicsScene->update();
	
	if (this->m_netEditable) {
		if (this->editItem(topItem)) {
			mouseEvent->accept();
			return true;
		}
	}
	else {
		//special case for jct mode
		if (topItem->type() == QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::JctItem) {
			this->initJctModeFunction((dynamic_cast<DJctItem*>(topItem))->getJctId());
			mouseEvent->accept();
			return true;
		}

		if (this->viewItem(topItem)) {
			mouseEvent->accept();
			return true;
		}
	}
	return false;
}

bool DBaseNaviFunction::mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	if (!this->m_ignoreEvents && this->flag_selectionChanged) {
		this->showConnectorsOfSelected();
		this->flag_selectionChanged = false;
	}
	return false;
}

bool DBaseNaviFunction::mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	if (!this->m_ignoreEvents && this->m_deleteMode) {
		this->ref_graphicsScene->clearSelection();

		// switching the cursor icon
		QGraphicsItem* item = this->ref_graphicsScene->itemAt(mouseEvent->scenePos(), {});
		if (item) {
			auto type = item->type();
			if (type != QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::CellItem 
				&& type != QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::CellConnectorItem) {
				item->setSelected(true);
				this->ref_graphicsView->viewport()->setCursor(QCursor(QPixmap("./icons/trashbin_25px.png")));

				//DESIGN why not CrossCursor??? JLo
				//Qt::CursorShape::CrossCursor

				mouseEvent->accept();
				return true;
			}
		}
		
		this->ref_graphicsView->viewport()->setCursor(Qt::ArrowCursor);
	}
	return false;
}
