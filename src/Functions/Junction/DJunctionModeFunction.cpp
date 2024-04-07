#include "DJunctionModeFunction.h"

#include "Scenario.h"
#include "Net_Container.h"
#include "Jct_Node.h"

#include "GraphicsItems/DCellItem.h"
#include "GraphicsItems/DDemandSinkCellItem.h"
#include "GraphicsItems/DJctItem.h"
#include "GraphicsItems/DPseudoJctLinkItem.h"

#include "PanelWidgets/Network/DArcPanel.h"
#include "PanelWidgets/Junction/DJctNodePanel.h"
#include "PanelWidgets/Junction/DJctInOutCellPanel.h"
#include "Functions/Junction/DJctInOutCellEditFunction.h"
#include "Functions/Junction/DJctLinkEditFunction.h"

#include "DFunctionHandler.h"
#include "GraphicsItems/DGraphicsScene.h"
#include "GraphicsItems/DGraphicsView.h"
#include "Functions/DBaseNaviFunction.h"
#include "DDockWidgetHandler.h"
#include "DTabToolBar.h"

#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>

const QColor DJunctionModeFunction::refColor_inOutCell = { 129, 66, 245 };

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DJunctionModeFunction::DJunctionModeFunction(int jctId) : QObject(nullptr), m_jctId(jctId) {
}

DJunctionModeFunction::~DJunctionModeFunction() {
}

void DJunctionModeFunction::initFunctionHandler() {
	//
	this->ref_baseNaviFunction->setIgnoreEvents(true);
	this->showViewPanel();

	//locks the graphics view to jct view
	this->fitJctInView();
	this->ref_graphicsView->setViewMoveable(false);
	QObject::connect(this->ref_graphicsView, &DGraphicsView::graphicsViewResized, this, &DJunctionModeFunction::fitJctInView);

	//hide the node
	this->ref_graphicsScene->getDJctItem(this->m_jctId)->setVisible(false);

	//re-color all the in out cells
	this->highlightInOutCells();
	
	//create all the pseudo-jctLinks
	this->ref_graphicsScene->removeAllDCellConnector();
	this->ref_graphicsScene->addAllDPseudoJLinkItemInJct(this->m_jctId);
	this->ref_graphicsScene->addAllJLinkItemInJct(this->m_jctId);
	this->ref_graphicsScene->setShowDetailJctLinkView(this->m_jctId, false);

	//some how setup the tabtool bar
	auto& jct = this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId);
	this->ref_tabToolBar->setJctTab(true, jct.getHasSig(), jct.getHasYellowBox());
	QObject::connect(this->ref_tabToolBar->getExitJctButton(), &QPushButton::clicked, this, &DJunctionModeFunction::exitJctMode);
}

int DJunctionModeFunction::getCurrentJctId() {
	return this->m_jctId;
}

// --- --- --- --- --- Slots --- --- --- --- ---

void DJunctionModeFunction::rightDockWidgetVisChanged() {
	if (!this->m_ignoreEvents && !this->ref_dockWidgetHandler->hasRightDockWidget())
		this->showViewPanel();
}

void DJunctionModeFunction::exitJctMode() {
	if (!this->m_ignoreEvents) {
		//handle all the things
		this->ref_dockWidgetHandler->removeAndDeleteRightDockWidget();
		this->ref_baseNaviFunction->setIgnoreEvents(false);
		this->ref_graphicsView->setViewMoveable(true);
		this->ref_tabToolBar->setJctTab(false);
		this->ref_graphicsScene->removeAllDPseudoJLinkItemInJct(this->m_jctId);
		this->ref_graphicsScene->removeAllJLinkItemInJct(this->m_jctId);
		this->ref_graphicsScene->getDJctItem(this->m_jctId)->setVisible(true);
		this->ref_graphicsScene->resetAllCellColor();
		this->ref_graphicsScene->update();
		//invoke dstr
		this->closeFunction();
	}
}

void DJunctionModeFunction::setIgnoreEvents(bool ignoresEvents) {
	this->m_ignoreEvents = ignoresEvents;
}

void DJunctionModeFunction::fitJctInView() {
	//get all the in/out cells of a junction
	auto& thisJct = this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId);
	auto thisJctItem = this->ref_graphicsScene->getDJctItem(this->m_jctId);

	//set initial to be the centre pt of jct node item
	qreal maxX = thisJctItem->pos().x();
	qreal minX = thisJctItem->pos().x();
	qreal maxY = thisJctItem->pos().y();
	qreal minY = thisJctItem->pos().y();

	//lambda for comp the bounding box of the jct
	auto doMaxMin = [&](QPointF& pt) {
		if (pt.x() > maxX)
			maxX = pt.x();
		if (pt.x() < minX)
			minX = pt.x();
		if (pt.y() > maxY)
			maxY = pt.y();
		if (pt.y() < minY)
			minY = pt.y();
	};

	//go through all the in out cells
	for (auto cellId : thisJct.getInCells()) {
		QGraphicsItem* cellItemPtr = this->ref_graphicsScene->getDCellItem(cellId);
		if (!cellItemPtr)
			cellItemPtr = this->ref_graphicsScene->getDmdSnkCellItem(cellId);

		if (cellItemPtr) {
			doMaxMin(cellItemPtr->mapToScene(cellItemPtr->boundingRect().topLeft()));
			doMaxMin(cellItemPtr->mapToScene(cellItemPtr->boundingRect().bottomRight()));
		}
	}
	for (auto cellId : thisJct.getOutCells()) {
		QGraphicsItem* cellItemPtr = this->ref_graphicsScene->getDCellItem(cellId);
		if (!cellItemPtr)
			cellItemPtr = this->ref_graphicsScene->getDmdSnkCellItem(cellId);

		if (cellItemPtr) {
			doMaxMin(cellItemPtr->mapToScene(cellItemPtr->boundingRect().topLeft()));
			doMaxMin(cellItemPtr->mapToScene(cellItemPtr->boundingRect().bottomRight()));
		}
	}

	//set the view
	this->ref_graphicsView->fitInView(minX, minY, (maxX - minX), (maxY - minY), Qt::AspectRatioMode::KeepAspectRatio);
}

// --- --- --- --- --- Private Utils --- --- --- --- ---

void DJunctionModeFunction::showViewPanel() {
	this->ref_dockWidgetHandler->setRightDockWidget(
		new DJctNodePanel(DJctNodePanel::Type_JctPanel::view, &(this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId))),
		"View Jct", true);
}

int DJunctionModeFunction::findJLinkId(QGraphicsItem* ptr) {
	switch (ptr->type()) {
	case (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::PseudoJLinkItem):
		for (auto& entry : this->ref_graphicsScene->getPseudoJctLinksInJct(this->m_jctId)) {
			if (entry.second == ptr)
				return entry.first;
		}
		break;

	case (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::JLinkItem):
		for (auto& entry : this->ref_graphicsScene->getJctLinksInJct(this->m_jctId)) {
			if (entry.second == ptr)
				return entry.first;
		}
		break;
	}
	return -1;
}

void DJunctionModeFunction::highlightInOutCells() {
	auto& jct = this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId);
	for (auto& cellId : jct.getInCells()) {
		auto cellItem = this->ref_graphicsScene->getDCellItem(cellId);
		if (cellItem) {
			cellItem->setUseCustomColor(true, this->refColor_inOutCell);
		}
		else {
			auto dmdCellItem = this->ref_graphicsScene->getDmdSnkCellItem(cellId);
			if (dmdCellItem)
				dmdCellItem->setUseCustomColor(true, this->refColor_inOutCell);
		}
	}
	for (auto& cellId : jct.getOutCells()) {
		auto cellItem = this->ref_graphicsScene->getDCellItem(cellId);
		if (cellItem) {
			cellItem->setUseCustomColor(true, this->refColor_inOutCell);
		}
		else {
			auto dmdCellItem = this->ref_graphicsScene->getDmdSnkCellItem(cellId);
			if (dmdCellItem)
				dmdCellItem->setUseCustomColor(true, this->refColor_inOutCell);
		}
	}

}

// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---

bool DJunctionModeFunction::keyPressEvent(QKeyEvent* keyEvent) {
	if (!this->m_ignoreEvents && keyEvent->key() == Qt::Key::Key_Escape) {
		keyEvent->accept();
		this->exitJctMode();
		return true;
	}
	return false;
}

bool DJunctionModeFunction::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	if (this->m_ignoreEvents || this->ref_graphicsScene->items(mouseEvent->scenePos()).isEmpty())
		return false;

	//grab the item that's clicked on
	auto* topItem = this->ref_graphicsScene->items(mouseEvent->scenePos()).first();
	this->ref_graphicsScene->clearSelection();

	switch (topItem->type()) {
	case (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::CellItem):
	{
		int cellId = (dynamic_cast<DCellItem*>(topItem))->getCellId();
		auto& jctNode = this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId);

		if (jctNode.getInCells().count(cellId) || jctNode.getOutCells().count(cellId)) {
			topItem->setSelected(true);
			this->ref_graphicsScene->setShowDetailJctLinkView(this->m_jctId, false);
			//simply addin the viewing panel
			this->ref_dockWidgetHandler->setRightDockWidget(
				new DJctInOutCellPanel{ DJctInOutCellPanel::Type_CellPanel::view,
					this->ref_scn->get()->getNetwork().getCell(cellId), jctNode,
					std::unordered_map<int, DPseudoJctLinkItem*>{this->ref_graphicsScene->getPseudoJctLinksInJct(this->m_jctId)} },
				"View In Out Cell", true);
			return true;
		}
		break;
	}
	case (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::DmdSnkCellItem):
	{
		int cellId = (dynamic_cast<DDemandSinkCellItem*>(topItem))->getCellId();
		auto& jctNode = this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId);

		if (jctNode.getInCells().count(cellId) || jctNode.getOutCells().count(cellId)) {
			topItem->setSelected(true);
			this->ref_graphicsScene->setShowDetailJctLinkView(this->m_jctId, false);
			//simply addin the viewing panel
			this->ref_dockWidgetHandler->setRightDockWidget(
				new DJctInOutCellPanel{ DJctInOutCellPanel::Type_CellPanel::view,
					this->ref_scn->get()->getNetwork().getCell(cellId), jctNode,
					std::unordered_map<int, DPseudoJctLinkItem*>{this->ref_graphicsScene->getPseudoJctLinksInJct(this->m_jctId)} },
				"View In Out Cell", true);
			return true;
		}
		break;
	}
	case (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::PseudoJLinkItem):
	{
		int jLinkId = this->findJLinkId(topItem);
		this->ref_graphicsScene->setShowDetailJctLinkView(this->m_jctId, false);
		this->ref_graphicsScene->setShowDetailJctLinkView(this->m_jctId, jLinkId, true);
		this->ref_graphicsScene->getJctLinksInJct(this->m_jctId).at(jLinkId)->setSelected(true);
		//add the viewing panel
		this->ref_dockWidgetHandler->setRightDockWidget(
			new DArcPanel{ DArcPanel::Type_ArcPanel::viewJL, this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId).getJctLink(jLinkId) },
			"View Junction Link", true);
		return true;
	}
	}

	return false;
}

bool DJunctionModeFunction::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	if (this->m_ignoreEvents || this->ref_graphicsScene->items(mouseEvent->scenePos()).isEmpty())
		return false;

	//grab the item that's clicked on
	this->ref_graphicsScene->clearSelection();

	for (auto* topItem : this->ref_graphicsScene->items(mouseEvent->scenePos())) {
		//loop so that it can find the JLink
		switch (topItem->type()) {
		case (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::CellItem):
		{
			int cellId = (dynamic_cast<DCellItem*>(topItem))->getCellId();
			auto& jctNode = this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId);

			if (jctNode.getInCells().count(cellId) || jctNode.getOutCells().count(cellId)) {
				topItem->setSelected(true);
				//invoke the cell editing function
				this->ref_functionHandler->addFunction(new DJctInOutCellEditFunction{ this->m_jctId, cellId });
				return true;
			}
			break;
		}
		case (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::DmdSnkCellItem):
		{
			int cellId = (dynamic_cast<DDemandSinkCellItem*>(topItem))->getCellId();
			auto& jctNode = this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId);

			if (jctNode.getInCells().count(cellId) || jctNode.getOutCells().count(cellId)) {
				topItem->setSelected(true);
				//invoke the cell editing function
				this->ref_functionHandler->addFunction(new DJctInOutCellEditFunction{ this->m_jctId, cellId });
				return true;
			}
			break;
		}
		case (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::PseudoJLinkItem):
			//there might be another JLink highlighted already
			this->ref_graphicsScene->setShowDetailJctLinkView(this->m_jctId, false);
			this->ref_graphicsScene->setShowDetailJctLinkView(this->m_jctId, this->findJLinkId(topItem), true);
		
		case (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::JLinkItem):
			//must do this as the PseudoJLinkItem is replaced with arc when clicked on

			int jLinkId = this->findJLinkId(topItem);
			if (jLinkId != -1) {
				this->ref_graphicsScene->getJctLinksInJct(this->m_jctId).at(jLinkId)->setSelected(true);
				//invoke the jct link editing function
				this->ref_functionHandler->addFunction(new DJctLinkEditFunction{ this->m_jctId, jLinkId });

				return true;
			}
			break;
		}
	}

	return false;
}




