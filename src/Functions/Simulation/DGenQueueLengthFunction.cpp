#include "DGenQueueLengthFunction.h"

#include <functional>
#include <iostream>
#include <fstream>

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
#include "PanelWidgets/Simulation/DGenQueueLengthPanel.h"
#include "GraphicsItems/DGraphicsScene.h"
#include "GraphicsItems/DCellItem.h"
#include "GraphicsItems/DDemandSinkCellItem.h"

// --- --- --- --- --- Colors --- --- --- --- ---

const QColor DGenQueueLengthFunction::refColor_tSelectStartColor{ Qt::green };
const QColor DGenQueueLengthFunction::refColor_tSelectEndColor{ Qt::red };
const QColor DGenQueueLengthFunction::refColor_selectedStartColor{ Qt::green };
const QColor DGenQueueLengthFunction::refColor_selectedEndColor{ Qt::red };

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DGenQueueLengthFunction::DGenQueueLengthFunction(const DISCO2_API::Rec_All* rec) : QObject(nullptr), I_Function(), ref_rec(rec) {}

DGenQueueLengthFunction::~DGenQueueLengthFunction() {}

void DGenQueueLengthFunction::initFunctionHandler() {
	//set up the graphics scene
	this->ref_graphicsScene->resetAllCellColor();
	this->ref_graphicsScene->resetAllCellText();
	for (auto jctId : this->ref_scn->get()->getNetwork().getJctIds())
		this->ref_graphicsScene->setShowDetailJctLinkView(jctId, false);

	//setup the panel widget
	this->m_panel = new DGenQueueLengthPanel{};
	this->ref_dockWidgetHandler->setRightDockWidget(this->m_panel, "Generate Queue Length");
	this->m_panel->connectFunction(this);
	this->m_panel->setRecStartEndTime(this->ref_rec->getStartTime(), 
		std::min(this->ref_rec->getEndTime(), (int)this->ref_rec->getAllRec().size() + this->ref_rec->getStartTime() - 1));

	//lock up tab tool bar
	this->ref_tabToolBar->setEnabled(false);
}

// --- --- --- --- --- Slots for Panel --- --- --- --- ---

void DGenQueueLengthFunction::cancel() {
	this->ref_graphicsScene->resetAllCellColor();
	this->ref_dockWidgetHandler->removeAndDeleteRightDockWidget();
	this->ref_tabToolBar->setEnabled(true);

	this->closeFunction();
}

void DGenQueueLengthFunction::reselectStartCell() {
	if (this->m_startCellId != -1) {
		//do color
		DCellItem* cellPtr = this->ref_graphicsScene->getDCellItem(this->m_startCellId);
		DDemandSinkCellItem* dmdCellPtr = this->ref_graphicsScene->getDmdSnkCellItem(this->m_startCellId);
		if (cellPtr) {
			cellPtr->setUseCustomColor(false);
			cellPtr->update();
		}
		else if (dmdCellPtr) {
			dmdCellPtr->setUseCustomColor(false);
			dmdCellPtr->update();
		}
		
		//record
		this->m_isSelectingStartCell = true;
		this->m_startCellId = -1;
		this->m_panel->setStartCellId(this->m_startCellId);
	}
}

void DGenQueueLengthFunction::removeEndCell(int cellId) {
	if (this->m_endCellIds.count(cellId)) {
		//record
		this->m_endCellIds.erase(cellId);
		this->m_panel->resetList(this->m_endCellIds);

		//do color
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
}

void DGenQueueLengthFunction::removeAllEndCell() {
	auto copy = this->m_endCellIds;
	for (auto& cellId : copy)
		this->removeEndCell(cellId);
}

void DGenQueueLengthFunction::confirmCellList() {
	//cal the queue for each time step

	//def temp struct to store search nodes
	struct TempNode {
		int cellId;
		double queueLength;
		std::list<int> fromCellIds;

		TempNode(int cellId, double queueLength, const std::unordered_map<int, double>& fromCellRef) 
				: cellId(cellId), queueLength(queueLength) {
			for (auto& entry : fromCellRef)
				this->fromCellIds.push_back(entry.first);
		}
	};

	//init
	std::map<int64_t, double> tQueueLengths{};
	const auto& thisEndCells = this->m_endCellIds;
	const auto& ref_allRec = this->ref_rec->getAllRec();
	auto& net = this->ref_scn->get()->getNetwork();

	//scrub input
	double thresholdRate = this->m_panel->getThresholdRate();
	int endTime = this->m_panel->getEndTime();
	if (endTime < 0)
		endTime = this->ref_rec->getEndTime();
	//adjust start end time to vector loc
	int64_t timeCounter = this->m_panel->getStartTime();
	for (auto itr = ref_allRec.begin() + std::max((int64_t)0, timeCounter - this->ref_rec->getStartTime());
		itr != ref_allRec.end() && timeCounter <= endTime; itr++) {
		//init this search
		int nextTest = this->m_startCellId;
		double queueLength = 0;
		double maxQueue = 0;
		std::unordered_set<int> beenToCellIds{};
		std::list<TempNode> divergeNodes{};

		//def checking lambdas
		std::function<bool()> toTestHasEndingFromCell = [&]() -> bool {
			auto& fromCells = net.getCell(nextTest)->getFromCellIds();
			for (auto& toTest : thisEndCells)
				if (fromCells.count(toTest))
					return true;
			return false;
		};
		std::function<bool()> isQueuing = [&]() -> bool {
			const DISCO2_API::Cell* cellPtr = net.getCell(nextTest);
			if ((*itr).count(nextTest) 
				&& (*itr).at(nextTest).at((int)DISCO2_API::I_RecordsHandler::recType::Occ) 
					> (thresholdRate * cellPtr->getStorageCapacity())) {
				return true;
			}
			return false;
		};


		//search from the queuestart cell
		do {
			//try simple loop
			while (isQueuing() && !toTestHasEndingFromCell() && net.getCell(nextTest)->getFromCellIds().size() == 1) {
				//add the queue
				queueLength += net.getCell(nextTest)->getFreeSpeed();
				nextTest = (*net.getCell(nextTest)->getFromCellIds().begin()).first;
			}

			//create div node
			if (isQueuing() && !toTestHasEndingFromCell()) {
				queueLength += net.getCell(nextTest)->getFreeSpeed();
				divergeNodes.push_back(TempNode(nextTest, queueLength, net.getCell(nextTest)->getFromCellIds()));
				nextTest = divergeNodes.back().fromCellIds.back();
				divergeNodes.back().fromCellIds.pop_back();
			}
			//this branch has ended
			else {
				maxQueue = std::max(maxQueue, queueLength);
				while (divergeNodes.size() && divergeNodes.back().fromCellIds.empty())
					divergeNodes.pop_back();
				//try next branch
				if (divergeNodes.size()) {
					nextTest = divergeNodes.back().fromCellIds.back();
					divergeNodes.back().fromCellIds.pop_back();
					queueLength = divergeNodes.back().queueLength;
				}
			}
		} while (divergeNodes.size());

		//store max queue
		tQueueLengths.emplace(timeCounter, maxQueue);
		timeCounter += this->ref_rec->getTimeStepSize();
	}

	//# file dialog to get path
	QString fileName = QFileDialog::getSaveFileName(nullptr, QObject::tr("Save Queue Length Results"), QDir::currentPath(), QObject::tr("CSV file (*.csv)"));
	if (!fileName.isEmpty()) {
		try {
			std::ofstream file;
			file.open(fileName.toStdString());
			if (file.is_open()) {
				file << "Time,Queue Length" << std::endl;

				for (auto& entry : tQueueLengths)
					file << std::to_string(entry.first) << ',' 
						<< std::to_string(entry.second) << std::endl;

				file.close();
			}
			else
				throw std::runtime_error("printOutput failed, File didn't open");

			this->cancel();
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

// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---

bool DGenQueueLengthFunction::keyPressEvent(QKeyEvent* keyEvent) {
	if (keyEvent->key() == Qt::Key::Key_Escape) {
		keyEvent->accept();
		this->cancel();
		return true;
	}

	return false;
}

bool DGenQueueLengthFunction::mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	//clear temp
	DCellItem* cellPtr = this->ref_graphicsScene->getDCellItem(this->temp_cellId);
	DDemandSinkCellItem* dmdCellPtr = this->ref_graphicsScene->getDmdSnkCellItem(this->temp_cellId);
	if (cellPtr) {
		cellPtr->setUseCustomColor(false);
		cellPtr->update();
	}
	else if (dmdCellPtr) {
		dmdCellPtr->setUseCustomColor(false);
		dmdCellPtr->update();
	}

	//find the new hover over item
	for (auto* topItem : this->ref_graphicsScene->items(mouseEvent->scenePos())) {
		switch (topItem->type()) {
		case (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::CellItem):
		{
			DCellItem* cellItem = dynamic_cast<DCellItem*>(topItem);
			this->temp_cellId = cellItem->getCellId();
			break;
		}
		case (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::DmdSnkCellItem):
		{
			DDemandSinkCellItem* dmdCellItem = dynamic_cast<DDemandSinkCellItem*>(topItem);
			this->temp_cellId = dmdCellItem->getCellId();
			break;
		}
		default:
			this->temp_cellId = -1;
			break;
		}

		//check for re-selection
		if (this->temp_cellId != -1 && !this->m_isSelectingStartCell
			&& (this->temp_cellId == this->m_startCellId || this->m_endCellIds.count(this->temp_cellId)))
				this->temp_cellId = -1;

		if (this->temp_cellId != -1) {
			//do color
			DCellItem* cellPtr = this->ref_graphicsScene->getDCellItem(this->temp_cellId);
			DDemandSinkCellItem* dmdCellPtr = this->ref_graphicsScene->getDmdSnkCellItem(this->temp_cellId);
			if (cellPtr) {
				cellPtr->setUseCustomColor(true, this->m_isSelectingStartCell ? this->refColor_tSelectStartColor : this->refColor_tSelectEndColor);
				cellPtr->update();
			}
			else if (dmdCellPtr) {
				dmdCellPtr->setUseCustomColor(true, this->m_isSelectingStartCell ? this->refColor_tSelectStartColor : this->refColor_tSelectEndColor);
				dmdCellPtr->update();
			}

			mouseEvent->accept();
			return true;
		}
	}

	return false;
}

bool DGenQueueLengthFunction::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	//cell must be already hovered over
	if (this->temp_cellId != -1) {

		if (this->m_isSelectingStartCell) {
			//set start cell
			this->m_startCellId = this->temp_cellId;

			//do color
			DCellItem* cellPtr = this->ref_graphicsScene->getDCellItem(this->temp_cellId);
			DDemandSinkCellItem* dmdCellPtr = this->ref_graphicsScene->getDmdSnkCellItem(this->temp_cellId);
			if (cellPtr){
				cellPtr->setUseCustomColor(true, this->refColor_selectedStartColor);
				cellPtr->update();
			}
			else if (dmdCellPtr) {
				dmdCellPtr->setUseCustomColor(true, this->refColor_selectedStartColor);
				dmdCellPtr->update();
			}
			this->m_panel->setStartCellId(this->m_startCellId);

			this->m_isSelectingStartCell = false;
		}
		else {
			//add end cell
			this->m_endCellIds.insert(this->temp_cellId);

			//do color
			DCellItem* cellPtr = this->ref_graphicsScene->getDCellItem(this->temp_cellId);
			DDemandSinkCellItem* dmdCellPtr = this->ref_graphicsScene->getDmdSnkCellItem(this->temp_cellId);
			if (cellPtr) {
				cellPtr->setUseCustomColor(true, this->refColor_selectedEndColor);
				cellPtr->update();
			}
			else if (dmdCellPtr) {
				dmdCellPtr->setUseCustomColor(true, this->refColor_selectedEndColor);
				dmdCellPtr->update();
			}

			//refresh end cell Ids
			this->m_panel->resetList(this->m_endCellIds);
		}

		this->temp_cellId = -1;
		mouseEvent->accept();
		return true;
	}

	return false;
}
