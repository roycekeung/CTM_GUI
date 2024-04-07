#include "DSplitCellFunction.h"

#include <QBrush>
#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>
#include <qmath.h>

#include "PanelWidgets/Network/DSplitCellPanel.h"
#include "GraphicsItems/DCellItem.h"
#include "GraphicsItems/DPolyArcItem.h"
#include "GraphicsItems/DGraphicsScene.h"
#include "Functions/DBaseNaviFunction.h"
#include "DTabToolBar.h"
#include "DDockWidgetHandler.h"

const double DSplitCellFunction::selectionBoxBuffer = 1;

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DSplitCellFunction::DSplitCellFunction() : QObject(nullptr), I_Function() {
}

DSplitCellFunction::~DSplitCellFunction() {
}

void DSplitCellFunction::initFunctionHandler() {
	//setup the panel widget
	this->m_panel = new DSplitCellPanel{};
	this->ref_dockWidgetHandler->setRightDockWidget(this->m_panel, "Split Cells");
	this->m_panel->setupButtons(this);
	this->m_panel->setShowing(false);

	//setup t_selectionBox
	this->t_selectionBox = new QGraphicsRectItem{};
	this->ref_graphicsScene->addItem(this->t_selectionBox);
	this->t_selectionBox->setVisible(false);	//init hide
	QPen tPen{};
	tPen.setColor(Qt::green);
	tPen.setCosmetic(true);
	tPen.setWidth(0);
	this->t_selectionBox->setPen(tPen);
	QColor tColor{ Qt::green };
	tColor.setAlpha(50);
	this->t_selectionBox->setBrush({ tColor });
	this->t_selectionBox->setZValue(DGraphicsScene::ZValue_GraphicItemLayer::ArcSpinePt);

	//make display clean
	this->ref_graphicsScene->removeAllDCellConnector_C2C();
	this->ref_graphicsScene->removeAllDCellConnector_C2J();

	//disable other stuff
	this->ref_baseNaviFunction->setIgnoreEvents(true);
	this->ref_tabToolBar->setEnabled(false);
}

// --- --- --- --- --- Slots for Panel --- --- --- --- ---

void DSplitCellFunction::cancel() {
	//remove temp from graphics scene
	this->cleanup();
	this->ref_graphicsScene->removeItem(this->t_selectionBox);
	delete this->t_selectionBox;
	
	//panel
	this->ref_dockWidgetHandler->removeAndDeleteRightDockWidget();

	//re-enable other stuff
	this->ref_baseNaviFunction->setIgnoreEvents(false);
	this->ref_tabToolBar->setEnabled(true);

	this->closeFunction();
}

void DSplitCellFunction::cancelSplit() {
	//revert graphics scene state
	this->cleanup();
	//reset internal state
	this->t_selectionBox->setVisible(false);
	this->m_hasSelectedRow = false;
	//reset panel
	this->m_panel->setShowing(false);
}

void DSplitCellFunction::splitsInputChanged(std::vector<int> splits) {
	this->redrawTempSplits(splits);
}

void DSplitCellFunction::confirmSplits() {
	//try core change
	try {
		this->ref_scn->get()->getNetEditor().arc_splitCells(this->m_selectedArcId, this->m_selectedRow, this->m_panel->getSplits());
	}
	catch (std::exception& e) {
		//print fail msg in the widget
		this->m_panel->failMessage({ e.what() });
		//early exit as operation failed
		return;
	}

	//revert graphics scene state
	this->cleanup();
	//figure out the change in core and update graphics
	std::vector<int> newRowVec = this->ref_scn->get()->getNetwork().getArc(this->m_selectedArcId)->getCellIdsInRow(this->m_selectedRow);
	std::unordered_set<int> newRow{ newRowVec.begin(), newRowVec.end() };
	for (auto& cellId : newRow)
		if (!this->ref_orgCellIds.count(cellId))
			this->ref_graphicsScene->addDCellItem(cellId);
	for (auto& cellId : this->ref_orgCellIds)
		if (!newRow.count(cellId))
			this->ref_graphicsScene->removeDCellItem(cellId);
	this->ref_graphicsScene->reloadLists();
	this->ref_graphicsScene->updateDArcItem(this->m_selectedArcId);
	//internal state
	this->m_hasSelectedRow = false;
	this->m_panel->setShowing(false);
}

// --- --- --- --- --- Private Utils --- --- --- --- ---

void DSplitCellFunction::cleanup() {
	for (auto& tCell : this->t_cells) {
		this->ref_graphicsScene->removeItem(tCell);
		delete tCell;
	}
	this->t_cells.clear();

	for (auto& cell : this->ref_orgCells)
		cell->setVisible(true);
	this->ref_orgCells.clear();
}

void DSplitCellFunction::redrawTempSplits(std::vector<int>& splits) {
	//remove all current
	for (auto& tCell : this->t_cells) {
		this->ref_graphicsScene->removeItem(tCell);
		delete tCell;
	}
	this->t_cells.clear();

	//rebuild
	auto* thisArc = this->ref_scn->get()->getNetwork().getArc(this->m_selectedArcId);
	auto* thisArcItem = this->ref_graphicsScene->getDArcItem(this->m_selectedArcId);
	double cellLength = thisArc->getVf() * this->ref_scn->get()->getNetwork().getSimTimeStepSize() / 1000;
	QPointF centerPt = thisArcItem->getRowCenterPt(this->m_selectedRow);
	qreal angleD = thisArcItem->getRowRot(this->m_selectedRow);
	qreal angleR = qDegreesToRadians(-angleD);
	int laneDone = 0;
	for (int& numOfLanes : splits) {
		DCellItem* tCellItem = new DCellItem{ -1, numOfLanes, cellLength };
		this->ref_graphicsScene->addItem(tCellItem);
		double thisW = -(0.5 * (numOfLanes - this->m_numOfLanes) + laneDone) * DGraphicsScene::laneWidth;
		tCellItem->setPos(centerPt + QPointF(-thisW * std::sin(angleR), -thisW * std::cos(angleR)));
		tCellItem->setRotation(angleD);
		tCellItem->update();
		this->t_cells.push_back(tCellItem);
		laneDone += numOfLanes;
	}
}

// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---

bool DSplitCellFunction::keyPressEvent(QKeyEvent* keyEvent) {
	if (keyEvent->key() == Qt::Key::Key_Escape) {
		keyEvent->accept();

		if (this->m_hasSelectedRow)
			this->cancelSplit();
		else
			this->cancel();

		return true;
	}

	return false;
}

bool DSplitCellFunction::mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	if (!this->m_hasSelectedRow) {
		mouseEvent->accept();

		for (auto* topItem : this->ref_graphicsScene->items(mouseEvent->scenePos()))
			if (topItem->type() == QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::ArcItem) {
				//get everything
				DPolyArcItem* arcItem = dynamic_cast<DPolyArcItem*>(topItem);
				this->m_selectedArcId = arcItem->getArcId();
				auto* thisArc = this->ref_scn->get()->getNetwork().getArc(this->m_selectedArcId);
				double cellLength = thisArc->getVf() * this->ref_scn->get()->getNetwork().getSimTimeStepSize() / 1000;
				this->m_selectedRow = arcItem->getRowClosestToPt(mouseEvent->scenePos());
				this->m_numOfLanes = thisArc->getNumOfLanes();
				if (this->m_selectedRow >= thisArc->getNumOfRows() || this->m_numOfLanes <= 1)
					continue;
				
				//draw selection box
				this->t_selectionBox->setVisible(true);
				this->t_selectionBox->setRect(
					-(cellLength * 0.5),
					-(this->ref_graphicsScene->laneWidth * this->m_numOfLanes * 0.5 + this->selectionBoxBuffer),
					cellLength,
					this->ref_graphicsScene->laneWidth * this->m_numOfLanes + this->selectionBoxBuffer * 2);
				this->t_selectionBox->setRotation(arcItem->getRowRot(this->m_selectedRow));
				this->t_selectionBox->setPos(arcItem->getRowCenterPt(this->m_selectedRow));
				this->t_selectionBox->update();

				return true;
			}

		//can't find any
		this->t_selectionBox->setVisible(false);
	}
	return false;
}

bool DSplitCellFunction::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	if (!this->m_hasSelectedRow && this->t_selectionBox->isVisible()) {
		mouseEvent->accept();

		this->m_hasSelectedRow = true;
		this->m_panel->setShowing(true);
		auto* thisArc = this->ref_scn->get()->getNetwork().getArc(this->m_selectedArcId);

		std::vector<int> splits{};
		this->ref_orgCells.clear();
		this->ref_orgCellIds.clear();
		for (auto& cellId : thisArc->getCellIdsInRow(this->m_selectedRow)) {
			this->ref_orgCellIds.insert(cellId);
			splits.push_back(this->ref_scn->get()->getNetwork().getCell(cellId)->getNumOfLanes());
			auto* cellItem = this->ref_graphicsScene->getDCellItem(cellId);
			this->ref_orgCells.push_back(cellItem);
			cellItem->setVisible(false);
		}

		this->m_panel->setParams(this->m_selectedArcId, this->m_selectedRow, this->m_numOfLanes, splits);
		this->redrawTempSplits(splits);

		return true;
	}
	return false;
}
