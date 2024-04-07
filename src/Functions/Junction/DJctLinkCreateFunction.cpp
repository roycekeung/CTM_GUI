#include "DJctLinkCreateFunction.h"

#include "Functions/DBaseNaviFunction.h"
#include "Functions/Junction/DJunctionModeFunction.h"
#include "PanelWidgets/Network/DArcPanel.h"
#include "DDockWidgetHandler.h"
#include "DTabToolBar.h"
#include "GraphicsItems/DGraphicsScene.h"
#include "GraphicsItems/DCellConnectorItem.h"
#include "GraphicsItems/DPseudoJctLinkItem.h"
#include "GraphicsItems/DCellItem.h"
#include "GraphicsItems/DDemandSinkCellItem.h"
#include "Utils/CoordTransform.h"
#include "Utils/DDefaultParamData.h"

#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>

#include "Jct_Link.h"
#include "VisInfo.h"
#include "Editor_Net.h"

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DJctLinkCreateFunction::DJctLinkCreateFunction() : QObject(nullptr) {
}

DJctLinkCreateFunction::~DJctLinkCreateFunction() {
}

void DJctLinkCreateFunction::initFunctionHandler() {
	//setup the panel widget
	this->m_panelWidet = new DArcPanel{ DArcPanel::Type_ArcPanel::createJL , nullptr, this->ref_dParam };
	this->ref_dockWidgetHandler->setRightDockWidget(this->m_panelWidet, "Create Junction Link");
	this->m_panelWidet->setupCreateJLButtons(this);

	//set up the graphics scene
	this->ref_baseNaviFunction->getJctModeFunction()->setIgnoreEvents(true);
	this->m_jctId = this->ref_baseNaviFunction->getJctModeFunction()->getCurrentJctId();
	this->ref_graphicsScene->setShowDetailJctLinkView(this->m_jctId, false);
	this->ref_graphicsScene->clearSelection();

	//lock up tab tool bar
	this->ref_tabToolBar->setEnabled(false);
}

// --- --- --- --- --- Checking and creation Functions --- --- --- --- ---

void DJctLinkCreateFunction::checkAndCreateJLink() {
	try {
		//create the arc
		int jctLinkId = this->ref_scn->get()->getNetEditor().jctLink_create(
			this->m_jctId, this->m_startCellId, this->m_endCellId,
			this->m_panelWidet->getLengthInput(), this->m_panelWidet->getQInput(), this->m_panelWidet->getKInput(),
			this->m_panelWidet->getVfInput(), this->m_panelWidet->getWInput()
		);

		// save newly amended inputted params to default data struct bakcup
		this->ref_dParam->m_JLinkFlowParams.NumberOfLane = this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId).getJctLink(jctLinkId)->getNumOfLanes();
		this->ref_dParam->m_JLinkFlowParams.FreeFlowSpeed = this->m_panelWidet->getVfInput();
		this->ref_dParam->m_JLinkFlowParams.BackwardShockwaveSpeed = this->m_panelWidet->getWInput();
		this->ref_dParam->m_JLinkFlowParams.JamDensity = this->m_panelWidet->getKInput();
		this->ref_dParam->m_JLinkFlowParams.SaturationFlow = this->m_panelWidet->getQInput();

		//have graphics scene make it
		this->ref_graphicsScene->clearSelection();
		this->ref_graphicsScene->removeAllDPseudoJLinkItemInJct(this->m_jctId);
		this->ref_graphicsScene->removeAllJLinkItemInJct(this->m_jctId);
		this->ref_graphicsScene->addAllDPseudoJLinkItemInJct(this->m_jctId);
		this->ref_graphicsScene->addAllJLinkItemInJct(this->m_jctId);
		this->ref_graphicsScene->update();
		this->ref_graphicsScene->setShowDetailJctLinkView(this->m_jctId, false);
		this->ref_graphicsScene->setShowDetailJctLinkView(this->m_jctId, jctLinkId, true);

		//close if succeeded
		this->cancel();
	}
	catch (std::exception& e) {
		//print fail msg in the widget
		this->m_panelWidet->failMessage({ e.what() });
	}
}

void DJctLinkCreateFunction::cancel() {
	//delete all the graphical things
	if (this->t_dragStLine) {
		//delete the temp cell connector standing in as pseudo arc
		this->ref_graphicsScene->removeItem(this->t_dragStLine);
		delete this->t_dragStLine;
	}
	if (this->t_pseudoJL) {
		this->ref_graphicsScene->removeItem(this->t_pseudoJL);
		delete this->t_pseudoJL;
	}

	//remove the arc panel
	this->ref_dockWidgetHandler->removeAndDeleteRightDockWidget();
	this->ref_baseNaviFunction->getJctModeFunction()->setIgnoreEvents(false);

	//reenable the tabToolBar
	this->ref_tabToolBar->setEnabled(true);

	// end this function
	this->closeFunction();
}

void DJctLinkCreateFunction::recalLength() {
	this->m_panelWidet->setLengthValue(this->t_pseudoJL->getLength());
}

// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---

bool DJctLinkCreateFunction::keyPressEvent(QKeyEvent* keyEvent) {
	if (keyEvent->key() == Qt::Key::Key_Escape) {
		keyEvent->accept();

		if (this->m_clickStage == createDrag) {
			this->m_clickStage = firstClick;
			
			//reset
			this->ref_graphicsScene->removeItem(this->t_dragStLine);
			delete this->t_dragStLine;
			this->t_dragStLine = nullptr;
		}
		else
			this->cancel();
		
		return true;
	}

	return false;
}

bool DJctLinkCreateFunction::mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	if (this->m_clickStage == createDrag) {
		//set the line
		this->t_dragStLine->setEndPt(mouseEvent->scenePos());
		mouseEvent->accept();
		return true;
	}
	return false;
}

bool DJctLinkCreateFunction::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	if (mouseEvent->button() == Qt::MouseButton::LeftButton) {
		switch (this->m_clickStage) {
		case firstClick:		//waiting for first click
		{						//from cell cannot be a dmd cell
			for (auto* topItem : this->ref_graphicsScene->items(mouseEvent->scenePos())) {
				if (topItem->type() == (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::CellItem)) {
					DCellItem* cellItem = dynamic_cast<DCellItem*>(topItem);

					//find the cellId
					if (this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId).getInCells().count(cellItem->getCellId())) {
						this->m_startCellId = cellItem->getCellId();
						cellItem->setSelected(true);
						//make the line
						this->t_dragStLine = new DCellConnectorItem{ cellItem->getConnectorExitPt(), mouseEvent->scenePos() };
						this->ref_graphicsScene->addItem(this->t_dragStLine);
						this->t_dragStLine->update();
						//change state
						this->m_clickStage = createDrag;

						break;
					}
				}
			}
			break;
		}
		case createDrag:	//waiting for second click
		{
			for (auto* topItem : this->ref_graphicsScene->items(mouseEvent->scenePos())) {
				if (topItem->type() == (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::CellItem)) {
					DCellItem* cellItem = dynamic_cast<DCellItem*>(topItem);

					//find the cellId
					if (this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId).getOutCells().count(cellItem->getCellId())) {
						this->m_endCellId = cellItem->getCellId();
						cellItem->setSelected(true);

						//swap cell connector with pseudo JLink
						this->t_dragStLine->setVisible(false);
						DCellItem* stCellItem = this->ref_graphicsScene->getDCellItem(this->m_startCellId);
						this->t_pseudoJL = new DPseudoJctLinkItem(
							stCellItem->getConnectorExitPt(), stCellItem->rotation(),
							cellItem->getConnectorEnterPt(), cellItem->rotation());
						this->ref_graphicsScene->addItem(this->t_pseudoJL);
						this->t_pseudoJL->update();

						//change state
						this->m_clickStage = createdFirstArc;
						this->m_panelWidet->setEditable(true);
						this->m_panelWidet->enableConfirmButton();
						this->recalLength();

						break;
					}
				}
				else if (topItem->type() == (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::DmdSnkCellItem)) {
					DDemandSinkCellItem* cellItem = dynamic_cast<DDemandSinkCellItem*>(topItem);

					//find the cellId
					if (this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId).getOutCells().count(cellItem->getCellId())) {
						this->m_endCellId = cellItem->getCellId();
						cellItem->setSelected(true);

						//swap cell connector with pseudo JLink
						this->t_dragStLine->setVisible(false);
						DCellItem* stCellItem = this->ref_graphicsScene->getDCellItem(this->m_startCellId);
						this->t_pseudoJL = new DPseudoJctLinkItem(
							stCellItem->getConnectorExitPt(), stCellItem->rotation(),
							cellItem->getConnectorEnterPt(), cellItem->rotation());
						this->ref_graphicsScene->addItem(this->t_pseudoJL);
						this->t_pseudoJL->update();

						//change state
						this->m_clickStage = createdFirstArc;
						this->m_panelWidet->setEditable(true);
						this->m_panelWidet->enableConfirmButton();
						this->recalLength();

						break;
					}
				}
			}
			break;
		}
		}
	}

	//intercepts all scene event so notthing else gets selected
	mouseEvent->accept();
	return true;
}
