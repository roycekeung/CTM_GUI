#include "DAddNewConnectorFunction.h"

// DISCO2_GUI lib
#include "PanelWidgets/DHintClosePanel.h"
#include "Functions/DBaseNaviFunction.h"
#include "GraphicsItems/DGraphicsScene.h"
#include "GraphicsItems/DCellConnectorItem.h"
#include "GraphicsItems/DCellItem.h"
#include "GraphicsItems/DDemandSinkCellItem.h"
#include "GraphicsItems/DJctItem.h"
#include "Utils/CoordTransform.h"
#include "DTabToolBar.h"
#include "DDockWidgetHandler.h"

// Qt lib
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>

// DISCO2_Core lib
#include "Editor_Net.h"

const std::string DAddNewConnectorFunction::hintText{
"Left click a cell to set as from cell \r\n\
left click another cell to set as to cell \r\n\
\r\n\
Holding shift will keep the same from cell\r\n\
\r\n"
};

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DAddNewConnectorFunction::DAddNewConnectorFunction() : QObject(nullptr) {}

DAddNewConnectorFunction::~DAddNewConnectorFunction() {}

void DAddNewConnectorFunction::initFunctionHandler() {
	// make a hint panel
	this->m_panel = new DHintClosePanel{ "Add Connectors" };
	this->m_panel->setHintText(this->hintText.c_str());
	this->ref_dockWidgetHandler->setRightDockWidget(this->m_panel);
	QObject::connect(this->m_panel->getCloseButton(), &QPushButton::clicked, this, &DAddNewConnectorFunction::cancel);

	//turn off stuff
	this->ref_baseNaviFunction->setIgnoreEvents(true);
	this->ref_tabToolBar->setEnabled(false);

	// ensure none is selected at first
	this->ref_graphicsScene->removeAllDCellConnector();
	this->ref_graphicsScene->clearSelection();
	this->ref_graphicsScene->addAllConnectorOfVisibleItems();
}

// --- --- --- --- --- Checking and creation Functions --- --- --- --- ---

void DAddNewConnectorFunction::checkAndCreateConnector() {
	try {
		if ((this->m_fromType == DGraphicsScene::ZValue_GraphicItemLayer::CellItem || this->m_fromType == DGraphicsScene::ZValue_GraphicItemLayer::DmdSnkCellItem)
			&& (this->m_toType == DGraphicsScene::ZValue_GraphicItemLayer::CellItem || this->m_toType == DGraphicsScene::ZValue_GraphicItemLayer::DmdSnkCellItem)) {
			this->ref_scn->get()->getNetEditor().cell_connectCells(this->m_startId, this->m_endId);
			this->ref_graphicsScene->addDCellConnector_C2C(this->m_startId, this->m_endId);
		}
		else if (this->m_fromType == DGraphicsScene::ZValue_GraphicItemLayer::JctItem 
			&& (this->m_toType == DGraphicsScene::ZValue_GraphicItemLayer::CellItem || this->m_toType == DGraphicsScene::ZValue_GraphicItemLayer::DmdSnkCellItem)) {
			this->ref_scn->get()->getNetEditor().jct_connectOutCell(this->m_startId, this->m_endId);
			this->ref_graphicsScene->reloadLists();
			this->ref_graphicsScene->addDCellConnector_C2J(this->m_endId, this->m_startId);
		}
		else if (this->m_fromType == DGraphicsScene::ZValue_GraphicItemLayer::CellItem && this->m_toType == DGraphicsScene::ZValue_GraphicItemLayer::JctItem) {
			this->ref_scn->get()->getNetEditor().jct_connectInCell(this->m_endId, this->m_startId);
			this->ref_graphicsScene->reloadLists();
			this->ref_graphicsScene->addDCellConnector_C2J(this->m_startId, this->m_endId);
		}
		else {
			//don't allow connection of dmd cell to jct || jct to jct
			this->m_panel->setFeedbackMessage("Invalid Connection");
		}
	}
	catch (std::exception& e) {
		this->m_panel->setFeedbackMessage(e.what());
	}
}

void DAddNewConnectorFunction::cancel() {

	this->ref_graphicsScene->removeAllDCellConnector();
	this->ref_baseNaviFunction->setIgnoreEvents(false);
	this->ref_dockWidgetHandler->removeAndDeleteRightDockWidget();

	// make sure the button isn't toggled
	this->ref_tabToolBar->setEnabled(true);

	this->closeFunction();
}

// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---

bool DAddNewConnectorFunction::keyPressEvent(QKeyEvent* keyEvent) {
	if (keyEvent->key() == Qt::Key::Key_Escape) {
		keyEvent->accept();

		//reset to initial
		if (this->m_clickStage == ClickStage::createDrag) {
			this->m_clickStage = ClickStage::firstClick;
			delete this->t_ConnectorItem;
		}
		else
			this->cancel();
		
		return true;
	}

	return false;
}

bool DAddNewConnectorFunction::mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	if (this->m_clickStage == createDrag) {
		//set moving line
		this->t_ConnectorItem->setEndPt(mouseEvent->scenePos());
		mouseEvent->accept();
		return true;
	}
	return false;
}

bool DAddNewConnectorFunction::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	if (mouseEvent->button() == Qt::MouseButton::LeftButton) {
		QPointF foundPt;

		for (auto* topItem : this->ref_graphicsScene->items(mouseEvent->scenePos())) {
			bool found = false;

			switch (topItem->type()) {
			case (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::CellItem):
			{
				DCellItem* cellItem = dynamic_cast<DCellItem*>(topItem);
				if (cellItem) {
					switch (this->m_clickStage) {
					case ClickStage::firstClick:
						this->m_startId = cellItem->getCellId();
						this->m_fromType = DGraphicsScene::ZValue_GraphicItemLayer::CellItem;
						foundPt = cellItem->getConnectorExitPt();
						break;
					case ClickStage::createDrag:
						this->m_endId = cellItem->getCellId();
						this->m_toType = DGraphicsScene::ZValue_GraphicItemLayer::CellItem;
						foundPt = cellItem->getConnectorExitPt();
						break;
					}

					found = true;
				}
				break;
			}
			case (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::DmdSnkCellItem):
			{
				DDemandSinkCellItem* cellItem = dynamic_cast<DDemandSinkCellItem*>(topItem);
				if (cellItem) {
					if (this->m_clickStage == ClickStage::firstClick && cellItem->getTypeOfCell() == DDemandSinkCellItem::Demand) {
						this->m_startId = cellItem->getCellId();
						this->m_fromType = DGraphicsScene::ZValue_GraphicItemLayer::DmdSnkCellItem;
						foundPt = cellItem->pos();
						found = true;
					}
					else if(this->m_clickStage == ClickStage::createDrag && cellItem->getTypeOfCell() == DDemandSinkCellItem::Sink){
						this->m_endId = cellItem->getCellId();
						this->m_toType = DGraphicsScene::ZValue_GraphicItemLayer::DmdSnkCellItem;
						foundPt = cellItem->pos();
						found = true;
					}
				}
				break;
			}
			case (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::JctItem):
			{
				DJctItem* jctItem = dynamic_cast<DJctItem*>(topItem);
				if (jctItem) {
					foundPt = jctItem->pos();

					switch (this->m_clickStage) {
					case ClickStage::firstClick:
						this->m_startId = jctItem->getJctId();
						this->m_fromType = DGraphicsScene::ZValue_GraphicItemLayer::JctItem;
						foundPt = jctItem->pos();
						break;
					case ClickStage::createDrag:
						this->m_endId = jctItem->getJctId();
						this->m_toType = DGraphicsScene::ZValue_GraphicItemLayer::JctItem;
						foundPt = jctItem->pos();
						break;
					}

					found = true;
				}
				break;
			}
			}

			if (found) {
				switch (this->m_clickStage) {
				case ClickStage::firstClick:
					//make temp connector
					this->t_ConnectorItem = new DCellConnectorItem{ foundPt, mouseEvent->scenePos() };
					this->ref_graphicsScene->addItem(this->t_ConnectorItem);
					this->m_clickStage = ClickStage::createDrag;
					break;
				case ClickStage::createDrag:
					//crate connector
					this->checkAndCreateConnector();
					//reset
					if (!mouseEvent->modifiers().testFlag(Qt::ShiftModifier)) {
						this->m_clickStage = firstClick;
						this->ref_graphicsScene->removeItem(this->t_ConnectorItem);
						delete this->t_ConnectorItem;
					}
					break;
				}
				break;
			}
		}
	}

	//intercepts all scene event so notthing else gets selected
	mouseEvent->accept();
	return true;
}
