#include "DGenDelayPlotFunction.h"

#include <unordered_set>

#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>

#include <Scenario.h>
#include <Net_Container.h>
#include <Cell.h>
#include <Rec_All.h>

#include "DTabToolBar.h"
#include "DDockWidgetHandler.h"
#include "PanelWidgets/Simulation/DGenDelayPlotPanel.h"
#include "GraphicsItems/DGraphicsScene.h"
#include "GraphicsItems/DCellItem.h"
#include "GraphicsItems/DDemandSinkCellItem.h"

// --- --- --- --- --- Colors --- --- --- --- ---

const QColor DGenDelayPlotFunction::refColor_tSelectColor = Qt::green;
const QColor DGenDelayPlotFunction::refColor_selectedColor = { 129, 66, 245 };

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DGenDelayPlotFunction::DGenDelayPlotFunction(const DISCO2_API::Rec_All* rec) : QObject(nullptr), I_Function(), ref_rec(rec) {
}

DGenDelayPlotFunction::~DGenDelayPlotFunction() {
}

void DGenDelayPlotFunction::initFunctionHandler() {
	//set up the graphics scene
	this->ref_graphicsScene->resetAllCellColor();
	this->ref_graphicsScene->resetAllCellText();
	for (auto jctId : this->ref_scn->get()->getNetwork().getJctIds())
		this->ref_graphicsScene->setShowDetailJctLinkView(jctId, false);

	//setup the panel widget
	this->m_panel = new DGenDelayPlotPanel{};
	this->ref_dockWidgetHandler->setRightDockWidget(this->m_panel, "Generate Plots");
	this->m_panel->connectFunction(this);
	this->m_panel->setRecStartEndTime(this->ref_rec->getStartTime(), 
		std::min(this->ref_rec->getEndTime(), (int)this->ref_rec->getAllRec().size() + this->ref_rec->getStartTime() - 1));

	//lock up tab tool bar
	this->ref_tabToolBar->setEnabled(false);
}

// --- --- --- --- --- Slots for Panel --- --- --- --- ---

void DGenDelayPlotFunction::cancel() {
	//del panel
	this->ref_dockWidgetHandler->removeAndDeleteRightDockWidget();

	//reset all the cells
	this->ref_graphicsScene->resetAllCellColor();
	this->ref_graphicsScene->resetAllCellText();
	for (auto jctId : this->ref_scn->get()->getNetwork().getJctIds())
		this->ref_graphicsScene->setShowDetailJctLinkView(jctId, false);

	//re-enable stuff
	this->ref_tabToolBar->setEnabled(true);

	this->closeFunction();
}

void DGenDelayPlotFunction::removeLast() {
	//reset cell color
	int cellId = this->m_cellList.back();
	DCellItem* cellPtr = this->ref_graphicsScene->getDCellItem(cellId);
	DDemandSinkCellItem* dmdCellPtr = this->ref_graphicsScene->getDmdSnkCellItem(cellId);
	if (cellPtr) {
		cellPtr->setUseCustomColor(false);
		cellPtr->setUseText(false);
		cellPtr->update();
	}
	else if (dmdCellPtr) {
		dmdCellPtr->setUseCustomColor(false);
		dmdCellPtr->setUseText(false);
		dmdCellPtr->update();
	}

	//records
	this->m_cellList.pop_back();
	this->temp_lastEndingCell = this->m_cellList.size() ? this->m_cellList.back() : -1;
	this->m_panel->removeLastRow();

	//reset JLs
	for (auto jctId : this->ref_scn->get()->getNetwork().getJctIds())
		this->ref_graphicsScene->setShowDetailJctLinkView(jctId, false);
	this->showNeededJctLinks(this->m_cellList);
}

void DGenDelayPlotFunction::removeAll() {
	//reset all the cells
	this->ref_graphicsScene->resetAllCellColor();
	this->ref_graphicsScene->resetAllCellText();
	for (auto jctId : this->ref_scn->get()->getNetwork().getJctIds())
		this->ref_graphicsScene->setShowDetailJctLinkView(jctId, false);

	//records
	this->m_cellList.clear();
	this->temp_lastEndingCell = -1;
	this->m_panel->removeAllRows();
}

void DGenDelayPlotFunction::confirmCellList() {
	//file dialog to get path
	QString fileName = QFileDialog::getSaveFileName(nullptr, QObject::tr("Save Plot"), QDir::currentPath(), QObject::tr("CSV file (*.csv)"));
	if (!fileName.isEmpty()) {
		try {
			if (this->m_panel->getPlotType() >= 0) {
				if (this->m_panel->getPlotType() < 5)
					//make the file
					this->ref_rec->printOutput(
						fileName.toStdString(),
						std::vector<int>{ this->m_cellList.begin(), this->m_cellList.end() },
						(DISCO2_API::I_RecordsHandler::recType)this->m_panel->getPlotType(),
						this->m_panel->getStartTime(), this->m_panel->getEndTime()
						);
				else if(this->m_panel->getPlotType() == 5)
					this->ref_rec->printOutput(fileName.toStdString(), 
						this->m_panel->getStartTime(), this->m_panel->getEndTime());

				this->cancel();
			}
		}
		catch (std::exception& e) {
			// print out corresponding error
			QMessageBox ErrormessageBox;
			QString Error_name = QString::fromStdString(e.what());
			ErrormessageBox.critical(nullptr, "Error", Error_name);
			ErrormessageBox.setFixedSize(500, 200);
		}
	}
}

// --- --- --- --- --- Private Utils --- --- --- --- ---

void DGenDelayPlotFunction::cleanupTemp() {
	//reset the temp cells
	for (auto& cellId : this->temp_cellList) {
		DCellItem* cellPtr = this->ref_graphicsScene->getDCellItem(cellId);
		DDemandSinkCellItem* dmdCellPtr = this->ref_graphicsScene->getDmdSnkCellItem(cellId);
		if (cellPtr) {
			cellPtr->setUseCustomColor(false);
			cellPtr->update();
		}
		else if (dmdCellPtr) {
			dmdCellPtr->setUseCustomColor(false);
			dmdCellPtr->update();
		}
	}

	//reset all the jct links
	for (auto jctId : this->ref_scn->get()->getNetwork().getJctIds())
		this->ref_graphicsScene->setShowDetailJctLinkView(jctId, false);
	this->showNeededJctLinks(this->m_cellList);

	//record
	this->temp_cellList.clear();
}

void DGenDelayPlotFunction::showTempCells(int endCell, bool isThisCellOnly) {
	//#1	find a list of cells
	//prep for lambda
	std::unordered_set<int> beenTo{ this->m_cellList.begin(), this->m_cellList.end() };
	std::list<int>& cellSeq = this->temp_cellList;
	const DISCO2_API::Net_Container& net = this->ref_scn->get()->getNetwork();

	//def recursive lambda to find cell seq
	std::function<bool(int)> findNext = [&](int nextTest)->bool{
		//temp insert itself so won't infinite loop
		beenTo.insert(nextTest);

		//reduce stack size, store single connections cells
		std::vector<int> tempList;
		while (net.getCell(nextTest)->getToCellIds().size() == 1 
			&& !net.getCell(nextTest)->getToCellIds().count(endCell)) {
			//get next
			nextTest = (*(net.getCell(nextTest)->getToCellIds().begin())).first;
			
			if (beenTo.count(nextTest))
				return false;
			
			tempList.push_back(nextTest);
			beenTo.insert(nextTest);
		}

		//try all to cells
		for (auto& toTest : net.getCell(nextTest)->getToCellIds()) {
			if ( (toTest.first == endCell) 
					|| (!beenTo.count(toTest.first) && findNext(toTest.first)) ) {
				cellSeq.push_front(toTest.first);
				for (auto itr = tempList.rbegin(); itr != tempList.rend(); itr++)
					cellSeq.push_front(*itr);
				return true;
			}
		}
		return false;
	};

	//do the search and add endCell iff can't find a seq
	if (isThisCellOnly || !(this->temp_lastEndingCell != -1 && findNext(this->temp_lastEndingCell)))
		cellSeq.push_back(endCell);
		

	//#2	highlight the list of cells
	//adjust cell seq for show Jct Links
	std::list<int> t_cellSeq = cellSeq;
	if (this->temp_lastEndingCell != -1)
		t_cellSeq.push_front(this->temp_lastEndingCell);
	this->showNeededJctLinks(t_cellSeq);
	//cell color
	for (auto& cellId : cellSeq) {
		DCellItem* cellItem = this->ref_graphicsScene->getDCellItem(cellId);
		DDemandSinkCellItem* dmdCellItem = this->ref_graphicsScene->getDmdSnkCellItem(cellId);
		if (cellItem) {
			cellItem->setUseCustomColor(true, this->refColor_tSelectColor);
			cellItem->update();
		}
		else if (dmdCellItem) {
			dmdCellItem->setUseCustomColor(true, this->refColor_tSelectColor);
			dmdCellItem->update();
		}
	}
}

void DGenDelayPlotFunction::showNeededJctLinks(std::list<int>& seq) {
	int tJctId = -1;	//keeps track which jct incell is
	for (auto& cellId : seq) {
		if (this->ref_graphicsScene->getRefJctInOutCellId().count(cellId))
			tJctId = this->ref_graphicsScene->getRefJctInOutCellId().at(cellId);

		if (!this->ref_graphicsScene->getRefCellIds().count(cellId) && tJctId != -1) {
			auto& jctNode = this->ref_scn->get()->getNetwork().getJctNode(tJctId);
			for (auto& jLinkId : jctNode.getJctLinkIds()) {
				if (jctNode.getJctLink(jLinkId)->getArcStartCellId() == cellId)
					this->ref_graphicsScene->setShowDetailJctLinkView(tJctId, jLinkId, true);
			}
		}
	}
}

void DGenDelayPlotFunction::addTempToList() {
	//get current index
	int index = this->m_cellList.size();

	//cell color
	for (auto& cellId : this->temp_cellList) {
		index++;
		DCellItem* cellItem = this->ref_graphicsScene->getDCellItem(cellId);
		DDemandSinkCellItem* dmdCellItem = this->ref_graphicsScene->getDmdSnkCellItem(cellId);
		if (cellItem) {
			cellItem->setUseCustomColor(true, this->refColor_selectedColor);
			cellItem->setUseText(true, QString::fromStdString(std::to_string(index)));
			cellItem->update();
		}
		else if (dmdCellItem) {
			dmdCellItem->setUseCustomColor(true, this->refColor_selectedColor);
			dmdCellItem->setUseText(true, QString::fromStdString(std::to_string(index)));
			dmdCellItem->update();
		}
	}

	//add the temp list to perm
	for (auto& cellId : this->temp_cellList) {
		this->m_cellList.push_back(cellId);
		this->m_panel->addRow(cellId);
	}
	this->temp_lastEndingCell = this->temp_cellList.back();
	this->temp_cellList.clear();
}

// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---

bool DGenDelayPlotFunction::keyPressEvent(QKeyEvent* keyEvent) {
	switch (keyEvent->key()) {
	case Qt::Key::Key_Escape:
		keyEvent->accept();
		this->cancel();
		return true;

	case Qt::Key::Key_Shift: 
		if(this->temp_cellList.size()){
			int cellId = this->temp_cellList.back();
			this->cleanupTemp();
			this->showTempCells(cellId, true);
			keyEvent->accept();
			return true;
		}
	}

	return false;
}

bool DGenDelayPlotFunction::keyReleaseEvent(QKeyEvent* keyEvent) {
	if (keyEvent->key() == Qt::Key::Key_Shift && this->temp_cellList.size()) {
		int cellId = this->temp_cellList.back();
		this->cleanupTemp();
		this->showTempCells(cellId, false);
		keyEvent->accept();
		return true;
	}

	return false;
}

bool DGenDelayPlotFunction::mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	//clean last
	this->cleanupTemp();

	for (auto* topItem : this->ref_graphicsScene->items(mouseEvent->scenePos())) {
		//find the hovering over cell id
		int cellId = -1;
		switch (topItem->type()) {
		case (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::CellItem):
		{
			DCellItem* cellItem = dynamic_cast<DCellItem*>(topItem);
			cellId = cellItem->getCellId();
			break;
		}
		case (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::DmdSnkCellItem):
		{
			DDemandSinkCellItem* dmdCellItem = dynamic_cast<DDemandSinkCellItem*>(topItem);
			cellId = dmdCellItem->getCellId();
			break;
		}
		}

		//check if is already selected
		if (cellId != -1) {
			for (auto& selectedCellId : this->m_cellList)
				if (selectedCellId == cellId) {
					cellId = -1;
					break;
				}
		}

		if (cellId != -1) {
			//find connecting cell seq
			this->showTempCells(cellId, mouseEvent->modifiers().testFlag(Qt::ShiftModifier));
			mouseEvent->accept();
			return true;
		}
	}

	//not finding any cells
	return false;
}

bool DGenDelayPlotFunction::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	if (this->temp_cellList.size()) {
		this->addTempToList();
		mouseEvent->accept();
		return true;
	}
	return false;
}
