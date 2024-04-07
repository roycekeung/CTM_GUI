#include "DGraphicsScene.h"

#include "DDemandSinkCellItem.h"
#include "DCellItem.h"
#include "DPolyArcItem.h"
#include "DJctItem.h"
#include "DCellConnectorItem.h"
#include "DPseudoJctLinkItem.h"
#include "DJctLinkItem.h"
#include "DMainWindow.h"

#include <VisInfo.h>

#include <qmath.h>

const double DGraphicsScene::laneWidth = 3.0;

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DGraphicsScene::DGraphicsScene(DMainWindow* mainWindow, std::shared_ptr<DISCO2_API::Scenario>* scn, CoordTransform* coordTransform) :
	QGraphicsScene(mainWindow), ref_scn(scn), ref_mainWindow(mainWindow), ref_coordTransform(coordTransform) {

	//init everything from the ref_scn
	this->reloadNet();
}

DGraphicsScene::~DGraphicsScene() {

}

// --- --- --- --- --- Getters --- --- --- --- ---

const std::unordered_set<int>& DGraphicsScene::getRefDmdsnkCellIds() const {
	return this->ref_dmdsnkCellIds;
}

const std::unordered_set<int>& DGraphicsScene::getRefCellIds() const {
	return this->ref_cellIds;
}

const std::unordered_set<int>& DGraphicsScene::getRefArcIds() const {
	return this->ref_arcIds;
}

const std::unordered_set<int>& DGraphicsScene::getRefJctIds() const {
	return this->ref_jctIds;
}

const std::unordered_set<int>& DGraphicsScene::getRefJctLinkArcIds() const {
	return this->ref_jctLinkArcIds;
}

const std::unordered_map<int, int>& DGraphicsScene::getRefJctInOutCellId() const {
	return this->ref_jctInOutCellId;
}

DCellItem* DGraphicsScene::getDCellItem(int cellId) {
	if (this->m_CellItems.count(cellId))
		return this->m_CellItems.at(cellId);
	return nullptr;
}

DPolyArcItem* DGraphicsScene::getDArcItem(int arcId) {
	if (this->m_ArcItems.count(arcId))
		return this->m_ArcItems.at(arcId);
	return nullptr;
}

DJctItem* DGraphicsScene::getDJctItem(int jctId) {
	if (this->m_JctItems.count(jctId))
		return this->m_JctItems.at(jctId);
	return nullptr;
}

DDemandSinkCellItem* DGraphicsScene::getDmdSnkCellItem(int cellId) {
	if (this->m_DmdSnkCellItems.count(cellId))
		return this->m_DmdSnkCellItems.at(cellId);
	return nullptr;
}


const std::unordered_map<int, DPseudoJctLinkItem*>& DGraphicsScene::getPseudoJctLinksInJct(int jctId) const {
	return this->m_PseudoJLinkItems.at(jctId);
}

const std::unordered_map<int, DJctLinkItem*>& DGraphicsScene::getJctLinksInJct(int jctId) const {
	return this->m_JLinkItems.at(jctId);
}

const std::unordered_map<int, DCellItem*>& DGraphicsScene::getJctLinkCellsInJct(int jctId, int jctLinkId) const {
	return this->m_JCellItems.at(jctId).at(jctLinkId);
}

DCellConnectorItem* DGraphicsScene::getDCellConnector_C2C(int fromCellId, int toCellId) {
	for (auto& rec : this->m_CellConnectorItems)
		if (rec.comp(fromCellId, toCellId))
			return rec.item;
	return nullptr;
}

std::unordered_map<int, DCellConnectorItem*> DGraphicsScene::getToCellConnectors(int fromCellId) {
	std::unordered_map<int, DCellConnectorItem*> out{};
	for (auto& rec : this->m_CellConnectorItems)
		if (rec.fromCellId == fromCellId)
			out.emplace(rec.toCellId, rec.item);

	return out;
}

std::unordered_map<int, DCellConnectorItem*> DGraphicsScene::getFromCellConnectors(int toCellId) {
	std::unordered_map<int, DCellConnectorItem*> out{};
	for (auto& rec : this->m_CellConnectorItems)
		if (rec.toCellId == toCellId)
			out.emplace(rec.fromCellId, rec.item);

	return out;
}

DGraphicsScene::CellConnectorEntry DGraphicsScene::getCellConnectorEntry_C2C(DCellConnectorItem* connector) const {
	for (auto& rec : this->m_CellConnectorItems)
		if (rec.item == connector)
			return rec;

	return CellConnectorEntry();
}

DGraphicsScene::CellJctConnectorEntry DGraphicsScene::getCellConnectorEntry_C2J(DCellConnectorItem* connector) const {
	for (auto& rec : this->m_CellJctConnectorItems)
		if (rec.item == connector)
			return rec;

	return CellJctConnectorEntry();
}

// --- --- --- --- --- Handling Graphic Items --- --- --- --- ---

void DGraphicsScene::reloadNet() {
	//clears all the exsisting items from scene
	for(auto thing : this->items())
		if (thing->type() != QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::BaseMap) {
			this->removeItem(thing);
			delete thing;
		}

	this->m_DmdSnkCellItems.clear();
	this->m_CellItems.clear();
	this->m_ArcItems.clear();
	this->m_JctItems.clear();
	this->m_JctLinkItems.clear();
	this->m_JCellItems.clear();
	this->m_JLinkItems.clear();
	this->m_CellConnectorItems.clear();
	this->m_CellJctConnectorItems.clear();
	//refresh the sorted item lists
	this->reloadLists();

	//add all the dmdSnk cells
	for (const int& dmdSnkCellId : this->ref_dmdsnkCellIds) {
		this->addDmdSnkItem(dmdSnkCellId);
	}
	//add all the arcs
	for (const int& arcId : this->ref_arcIds)
		this->addDArcItem(arcId);
	//add all the cells
	for (const int& cellId : this->ref_cellIds)
		this->addDCellItem(cellId);
	//add all the jcts
#ifndef LOCKED_GUI_DEPLOY
	for (const int& jctId : this->ref_jctIds)
		this->addDJctItem(jctId);
#else
	//create jct limit 3
	int jctCount = 0;
	for (const int& jctId : this->ref_jctIds) {
		if (jctCount < 3) {
			this->addDJctItem(jctId);
			++jctCount;
		}
		else {
			try {
				this->ref_scn->get()->getNetEditor().jct_deleteJunctNode(jctId);
			}
			catch (...) {

			}
		}
	}
#endif
	
}

void DGraphicsScene::reloadLists() {
	const auto& net = this->ref_scn->get()->getNetwork();
	//dmdCellsIds
	this->ref_dmdsnkCellIds = net.getDemandSinkCellIds();
	//jctIds
	this->ref_jctIds = net.getJctIds();
	//jctLinkArcIds / and collect cellIds
	std::unordered_set<int> jctLinkCellIds{};
	this->ref_jctLinkArcIds.clear();
	this->ref_jctInOutCellId.clear();
	for (auto& jctId : net.getJctIds()) {
		const auto& jct = net.getJctNode(jctId);
		//jctArc & jctLink Cell ids
		for (auto& jLinkId : jct.getJctLinkIds()) {
			const auto* jLink = jct.getJctLink(jLinkId);
			this->ref_jctLinkArcIds.insert(jLink->getId());
			for (auto& row : jLink->getCellIds()) {
				for (auto& id : row)
					jctLinkCellIds.insert(id);
			}
		}
		//jct inOut cell ids
		for (auto& cellId : jct.getInCells())
			this->ref_jctInOutCellId.emplace(cellId, jctId);
		for (auto& cellId : jct.getOutCells())
			this->ref_jctInOutCellId.emplace(cellId, jctId);
	}
	//arcIds
	this->ref_arcIds.clear();
	for (auto& arcId : net.getArcIds()) {
		if (!this->ref_jctLinkArcIds.count(arcId))
			this->ref_arcIds.insert(arcId);
	}
	//cellIds
	this->ref_cellIds.clear();
	for (auto& cellId : net.getCellIds()) {
		if (!jctLinkCellIds.count(cellId) && !this->ref_dmdsnkCellIds.count(cellId))
			this->ref_cellIds.insert(cellId);
	}
}

DCellItem* DGraphicsScene::addDCellItem(int cellId) {
	//make sure function no throw
	if (this->ref_scn->get()->getNetwork().getCellIds().count(cellId) && !this->m_CellItems.count(cellId)) {
		//create a dummy thing
		DCellItem* cellItem = new DCellItem(cellId);
		//put into things
		this->m_CellItems.emplace(cellId, cellItem);
		this->addItem(cellItem);
		emit itemsChanged();

		//update the size and location
		this->updateDCellItem(cellId);
		return cellItem;
	}
	return nullptr;
}

DJctItem* DGraphicsScene::addDJctItem(int jctId) {
	//make sure function no throw
	if (this->ref_scn->get()->getNetwork().getJctIds().count(jctId) && !this->m_JctItems.count(jctId)) {
		//create a dummy thing
		DJctItem* jctItem = new DJctItem(jctId);
		//put into things
		this->m_JctItems.emplace(jctId, jctItem);
		this->addItem(jctItem);
		emit itemsChanged();

		//update the size and location
		this->updateDJctItem(jctId);
		return jctItem;
	}
	return nullptr;
}

DPolyArcItem* DGraphicsScene::addDArcItem(int arcId) {
	//Make sure function no throw
	if (this->ref_scn->get()->getNetwork().getArcIds().count(arcId) && !this->m_ArcItems.count(arcId)) {
		//Get all the things
		const DISCO2_API::Arc* thisArc = this->ref_scn->get()->getNetwork().getArc(arcId);

		//create a dummy thing
		DPolyArcItem* arcItem = new DPolyArcItem(arcId);
		//put into things
		this->m_ArcItems.emplace(arcId, arcItem);
		this->addItem(arcItem);
		emit itemsChanged();

		//load all the cells as well, addDCellItem performs checks on cell creation anyways
		for (auto& row : thisArc->getCellIds())
			for (auto& cellId : row)
				this->addDCellItem(cellId);

		//update the size and location
		this->updateDArcItem(arcId);
		return arcItem;
	}
	return nullptr;
}

DDemandSinkCellItem* DGraphicsScene::addDmdSnkItem(int cellId) {
	// true if current cellId exists in core and havent been recorded nor created in graphicscene 
	if (this->ref_scn->get()->getNetwork().getDemandSinkCellIds().count(cellId) && !this->m_DmdSnkCellItems.count(cellId)) {
		//create a dummy thing
		DDemandSinkCellItem* DmdSnkcellItem = new DDemandSinkCellItem(cellId, this->ref_scn->get()->getNetwork().getCell_DemandSink(cellId)->getNumOfLanes());
		// getFlowInCapacity == 0  means no flow in, does flow out; thus demand cell;  Type="DmdSnk" InQ="0"
		// getFlowInCapacity != 0  means does flow in, no flow out; thus Sink cell;   Type="DmdSnk" OutQ="0"
		if (this->ref_scn->get()->getNetwork().getCell_DemandSink(cellId)->getFlowInCapacity() == 0) {
			DmdSnkcellItem->setTypeOfCell(DDemandSinkCellItem::CellType::Demand);  // used for setting color
		}
		else {
			DmdSnkcellItem->setTypeOfCell(DDemandSinkCellItem::CellType::Sink);  // used for setting color
		}
		//put into records in graphicscene 
		this->m_DmdSnkCellItems.emplace(cellId, DmdSnkcellItem);
		this->addItem(DmdSnkcellItem);
		emit itemsChanged();

		//update the size and location
		this->updateDmdSnkItem(cellId);
		return DmdSnkcellItem;
	}
	return nullptr;
}

DCellConnectorItem* DGraphicsScene::addDCellConnector_C2C(int fromCellId, int toCellId) {
	if (this->ref_scn->get()->getNetwork().getCellIds().count(fromCellId)
			&& this->ref_scn->get()->getNetwork().getCellIds().count(toCellId)
			&& (this->m_CellItems.count(fromCellId) || this->m_DmdSnkCellItems.count(fromCellId))
			&& (this->m_CellItems.count(toCellId) || this->m_DmdSnkCellItems.count(toCellId))
			//check is actually connected
			&& this->ref_scn->get()->getNetwork().getCell(fromCellId)->getToCellIds().count(toCellId)) {
		
		//make sure not already made
		bool isMade = false;
		for (auto& entry : this->m_CellConnectorItems) {
			if (entry.comp(fromCellId, toCellId)) {
				isMade = true;
				break;
			}
		}

		//make if ok
		if (!isMade) {
			DCellConnectorItem* connectorItem = new DCellConnectorItem{
				(this->m_CellItems.count(fromCellId) ? this->m_CellItems.at(fromCellId)->getConnectorExitPt() : this->m_DmdSnkCellItems.at(fromCellId)->pos()),
				(this->m_CellItems.count(toCellId) ? this->m_CellItems.at(toCellId)->getConnectorEnterPt() : this->m_DmdSnkCellItems.at(toCellId)->pos()) };
			this->addItem(connectorItem);
			this->m_CellConnectorItems.emplace_back(CellConnectorEntry{ fromCellId, toCellId, connectorItem });
			return connectorItem;
		}
	}
	return nullptr;
}

DCellConnectorItem* DGraphicsScene::addDCellConnector_C2J(int cellId, int jctId) {
		if (this->ref_scn->get()->getNetwork().getJctIds().count(jctId)
			&& this->ref_scn->get()->getNetwork().getCellIds().count(cellId)
			&& (this->m_CellItems.count(cellId) || this->m_DmdSnkCellItems.count(cellId) )
			&& this->m_JctItems.count(jctId)) {

		//make sure not already made
		bool isMade = false;
		for (auto& entry : this->m_CellJctConnectorItems)
			if (entry.comp(jctId, cellId)) {
				isMade = true;
				break;
			}

		//make if ok
		if (!isMade) {
			//get all the things
			const auto& thisJct = this->ref_scn->get()->getNetwork().getJctNode(jctId);

			//check if cell is in or out or not connected at all
			DCellConnectorItem* connectorItem;
			if (thisJct.getInCells().count(cellId)) {
				QPointF pt = (this->m_CellItems.count(cellId) ? this->m_CellItems.at(cellId)->getConnectorExitPt() : this->m_DmdSnkCellItems.at(cellId)->pos());
				connectorItem = new DCellConnectorItem{ pt,	this->m_JctItems.at(jctId)->getEdgeIntersectionPoint(pt) };
			}
			else if (thisJct.getOutCells().count(cellId)) {
				QPointF pt = (this->m_CellItems.count(cellId) ? this->m_CellItems.at(cellId)->getConnectorEnterPt() : this->m_DmdSnkCellItems.at(cellId)->pos());
				connectorItem = new DCellConnectorItem{ this->m_JctItems.at(jctId)->getEdgeIntersectionPoint(pt), pt };
			}
			else
				return nullptr;

			this->addItem(connectorItem);
			this->m_CellJctConnectorItems.emplace_back(CellJctConnectorEntry{ jctId, cellId, connectorItem });
			return connectorItem;
		}
	}
		return nullptr;
}

void DGraphicsScene::addAllDPseudoJLinkItemInJct(int jctId) {
	if (this->ref_scn->get()->getNetwork().getJctIds().count(jctId)
		&& !this->m_PseudoJLinkItems.count(jctId)) {
		//make the map
		this->m_PseudoJLinkItems.emplace(jctId, std::unordered_map<int, DPseudoJctLinkItem*>{});

		auto& jctNode = this->ref_scn->get()->getNetwork().getJctNode(jctId);
		for (auto& jctLinkId : jctNode.getJctLinkIds()) {
			auto jctLink = jctNode.getJctLink(jctLinkId);

			int fromCellId = jctLink->getFromCellId();
			int toCellId = jctLink->getToCellId();

			//check cells exists
			if ((this->m_CellItems.count(fromCellId) || this->m_DmdSnkCellItems.count(fromCellId))
				&& (this->m_CellItems.count(toCellId) || this->m_DmdSnkCellItems.count(toCellId))) {
				//make the item
				DPseudoJctLinkItem* item = new DPseudoJctLinkItem(
					(this->m_CellItems.count(fromCellId) ? this->m_CellItems.at(fromCellId)->getConnectorExitPt() : this->m_DmdSnkCellItems.at(fromCellId)->pos()), 
					(this->m_CellItems.count(fromCellId) ? this->m_CellItems.at(fromCellId)->rotation() : this->m_DmdSnkCellItems.at(fromCellId)->rotation()), 
					(this->m_CellItems.count(toCellId) ? this->m_CellItems.at(toCellId)->getConnectorEnterPt() : this->m_DmdSnkCellItems.at(toCellId)->pos()),
					(this->m_CellItems.count(toCellId) ? this->m_CellItems.at(toCellId)->rotation() : this->m_DmdSnkCellItems.at(toCellId)->rotation()));
				this->m_PseudoJLinkItems.at(jctId).emplace(jctLinkId, item);
				this->addItem(item);
			}
		}
	}
}

void DGraphicsScene::addAllJLinkItemInJct(int jctId) {
	if (this->ref_scn->get()->getNetwork().getJctIds().count(jctId)
		&& !this->m_JLinkItems.count(jctId)) {
		//make the map
		this->m_JLinkItems.emplace(jctId, std::unordered_map<int, DJctLinkItem*>{});
		this->m_JCellItems.emplace(jctId, std::unordered_map<int, std::unordered_map<int, DCellItem*>>{});

		auto& jctNode = this->ref_scn->get()->getNetwork().getJctNode(jctId);
		for (auto& jctLinkId : jctNode.getJctLinkIds()) {
			auto jctLink = jctNode.getJctLink(jctLinkId);
			int fromCellId = jctLink->getFromCellId();
			int toCellId = jctLink->getToCellId();

			//make the map
			this->m_JCellItems.at(jctId).emplace(jctLinkId, std::unordered_map<int, DCellItem*>{});

			//check cells exists
			if ((this->m_CellItems.count(fromCellId) || this->m_DmdSnkCellItems.count(fromCellId))
					&& (this->m_CellItems.count(toCellId) || this->m_DmdSnkCellItems.count(toCellId))) {

				//make the JctLinkItem
				QPointF fromPt = this->m_CellItems.count(fromCellId) ? this->m_CellItems.at(fromCellId)->getConnectorExitPt() : this->m_DmdSnkCellItems.at(fromCellId)->pos();
				qreal fromRot = this->m_CellItems.count(fromCellId) ? this->m_CellItems.at(fromCellId)->rotation() : this->m_DmdSnkCellItems.at(fromCellId)->rotation();
				QPointF toPt = this->m_CellItems.count(toCellId) ? this->m_CellItems.at(toCellId)->getConnectorEnterPt() : this->m_DmdSnkCellItems.at(toCellId)->pos();
				qreal toRot = this->m_CellItems.count(toCellId) ? this->m_CellItems.at(toCellId)->rotation() : this->m_DmdSnkCellItems.at(toCellId)->rotation();
				DJctLinkItem* jctLinkItem = new DJctLinkItem{ jctLink->getId(), jctLinkId, fromPt, fromRot, toPt, toRot, jctLink->getNumOfLanes() };

				//make the cells
				std::vector<QGraphicsItem*> tmp_JctLinkInternalCells{};
				for (auto& cellId : jctLink->getAllCellIds()) {
					DCellItem* cellItem = this->addDCellItem(cellId);
					if (cellItem) {
						this->m_JCellItems.at(jctId).at(jctLinkId).emplace(cellId, cellItem);
						cellItem->setZValue(ZValue_GraphicItemLayer::JLinkItem + 1);
						tmp_JctLinkInternalCells.push_back(cellItem);
					}
				}

				//deal with record
				if (jctLinkItem) {
					this->m_JLinkItems.at(jctId).emplace(jctLinkId, jctLinkItem);
					this->addItem(jctLinkItem);
					emit itemsChanged();
					// set the pos of internal cells
					jctLinkItem->setItemPosAndRotation(tmp_JctLinkInternalCells);
				}
			}
		}
	}
}

void DGraphicsScene::removeDCellItem(int cellId) {
	if (this->m_CellItems.count(cellId)) {
		//pop the item
		DCellItem* cellItem = this->m_CellItems.at(cellId);
		this->m_CellItems.erase(cellId);

		//remove from scene and delete
		this->removeItem(cellItem);
		delete cellItem;
		emit itemsChanged();
	}
}

void DGraphicsScene::removeDJctItem(int jctId) {
	if (this->m_JctItems.count(jctId)) {
		//pop the item
		DJctItem* jctItem = this->m_JctItems.at(jctId);
		this->m_JctItems.erase(jctId);

		//remove from scene and delete
		this->removeItem(jctItem);
		delete jctItem;
		emit itemsChanged();
	}
}

void DGraphicsScene::removeDArcItem(int arcId) {
	if (this->m_ArcItems.count(arcId)) {
		//pop the item
		DPolyArcItem* arcItem = this->m_ArcItems.at(arcId);
		this->m_ArcItems.erase(arcId);

		//remove from scene and delete
		this->removeItem(arcItem);
		delete arcItem;
		emit itemsChanged();

		//remove all the cells inside
		for (auto& row : this->ref_scn->get()->getNetwork().getArc(arcId)->getCellIds())
			for (auto& cellId : row)
				this->removeDCellItem(cellId);
	}
}

void DGraphicsScene::removeDmdSnkItem(int cellId) {
	if (this->m_DmdSnkCellItems.count(cellId)) {
		//pop the item
		DDemandSinkCellItem* cellItem = this->m_DmdSnkCellItems.at(cellId);
		this->m_DmdSnkCellItems.erase(cellId);

		//remove from scene and delete
		this->removeItem(cellItem);
		delete cellItem;
		emit itemsChanged();
	}
}

void DGraphicsScene::addConnectorsOfSelectedItems() {
	//temp things to keep track
	std::unordered_set<int> thisSelectedCells{};
	std::unordered_set<int> thisSelectedJcts{};

	//go through all the items to see the selected cells
	for (auto* item : this->selectedItems()) {
		switch (item->type()) {
		case (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::ArcItem):
			for (auto& row : this->ref_scn->get()->getNetwork().getArc(dynamic_cast<DPolyArcItem*>(item)->getArcId())->getCellIds())
				for (auto& cellId : row) {
					thisSelectedCells.insert(cellId);
					if (this->m_CellItems.count(cellId)) {
						this->m_CellItems.at(cellId)->setSelected(true);
						this->m_CellItems.at(cellId)->update();
					}
				}
			break;

		case (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::CellItem):
			thisSelectedCells.insert(dynamic_cast<DCellItem*>(item)->getCellId());
			break;

		case (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::DmdSnkCellItem):
			thisSelectedCells.insert(dynamic_cast<DDemandSinkCellItem*>(item)->getCellId());
			break;

		case (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::JctItem):
			thisSelectedJcts.insert(dynamic_cast<DJctItem*>(item)->getJctId());
			break;
		}
	}

	//there are checks in the create C2C function so can spam create
	//NOTE: the checking is linear though so might be slow
	for (auto& cellId : thisSelectedCells) {
		const auto* cell = this->ref_scn->get()->getNetwork().getCell(cellId);
		for (auto& fromCell : cell->getFromCellIds())
			this->addDCellConnector_C2C(fromCell.first, cellId);
		for (auto& toCell : cell->getToCellIds())
			this->addDCellConnector_C2C(cellId, toCell.first);
		if(this->ref_jctInOutCellId.count(cellId))
			this->addDCellConnector_C2J(cellId, this->ref_jctInOutCellId.at(cellId));
	}

	for (auto& jctId : thisSelectedJcts) {
		const auto& jct = this->ref_scn->get()->getNetwork().getJctNode(jctId);
		for (auto& inCell : jct.getInCells())
			this->addDCellConnector_C2J(inCell, jctId);
		for (auto& outCell : jct.getOutCells())
			this->addDCellConnector_C2J(outCell, jctId);
	}
}

void DGraphicsScene::addAllConnectorOfVisibleItems() {

	for (auto& cellEntry : this->m_CellItems) {
		const auto* cell = this->ref_scn->get()->getNetwork().getCell(cellEntry.first);
		for (auto& fromCell : cell->getFromCellIds())
			this->addDCellConnector_C2C(fromCell.first, cellEntry.first);
		for (auto& toCell : cell->getToCellIds())
			this->addDCellConnector_C2C(cellEntry.first, toCell.first);
		if(this->ref_jctInOutCellId.count(cellEntry.first))
			this->addDCellConnector_C2J(cellEntry.first, this->ref_jctInOutCellId.at(cellEntry.first));
	}

	for (auto& cellEntry : this->m_DmdSnkCellItems) {
		const auto* cell = this->ref_scn->get()->getNetwork().getCell(cellEntry.first);
		for (auto& fromCell : cell->getFromCellIds())
			this->addDCellConnector_C2C(fromCell.first, cellEntry.first);
		for (auto& toCell : cell->getToCellIds())
			this->addDCellConnector_C2C(cellEntry.first, toCell.first);
		if (this->ref_jctInOutCellId.count(cellEntry.first))
			this->addDCellConnector_C2J(cellEntry.first, this->ref_jctInOutCellId.at(cellEntry.first));
	}
}

void DGraphicsScene::removeDCellConnector_C2C(int fromCellId, int toCellId) {
	for (auto it = this->m_CellConnectorItems.begin(); it != this->m_CellConnectorItems.end(); ++it) {
		if (it->comp(fromCellId, toCellId)) {
			this->removeItem(it->item);
			delete it->item;
			this->m_CellConnectorItems.erase(it);
			break;
		}
	}
}

void DGraphicsScene::removeDCellConnector_C2J(int cellId, int jctId) {
	for (auto it = this->m_CellJctConnectorItems.begin(); it != this->m_CellJctConnectorItems.end(); ++it) {
		if (it->comp(jctId, cellId)) {
			this->removeItem(it->item);
			delete it->item;
			this->m_CellJctConnectorItems.erase(it);
			break;
		}
	}
}

void DGraphicsScene::removeAllDPseudoJLinkItemInJct(int jctId) {
	if (this->m_PseudoJLinkItems.count(jctId)) {
		for (auto& entry : this->m_PseudoJLinkItems.at(jctId)) {
			this->removeItem(entry.second);
			delete entry.second;
		}
		this->m_PseudoJLinkItems.erase(jctId);
	}
}

void DGraphicsScene::removeAllJLinkItemInJct(int jctId) {
	//deal with the arcs first
	if (this->m_JLinkItems.count(jctId)) {
		for (auto& entry : this->m_JLinkItems.at(jctId)) {
			this->removeItem(entry.second);
			delete entry.second;
		}
		this->m_JLinkItems.erase(jctId);
	}
	//deal with the cells
	if (this->m_JCellItems.count(jctId)) {
		for (auto& JLEntry : this->m_JCellItems.at(jctId))
			for (auto& JCellEntry : JLEntry.second)
				this->removeDCellItem(JCellEntry.first);
		this->m_JCellItems.erase(jctId);
	}
}

void DGraphicsScene::removeAllDCellConnector_C2C() {
	for (auto& it : this->m_CellConnectorItems) {
		this->removeItem(it.item);
		delete it.item;
	}
	this->m_CellConnectorItems.clear();
}

void DGraphicsScene::removeAllDCellConnector_C2J() {
	for (auto& it : this->m_CellJctConnectorItems) {
		this->removeItem(it.item);
		delete it.item;
	}
	this->m_CellJctConnectorItems.clear();
}

void DGraphicsScene::removeAllDCellConnector() {
	this->removeAllDCellConnector_C2C();
	this->removeAllDCellConnector_C2J();
}

void DGraphicsScene::updateDCellItem(int cellId) {
	//make sure function no throw
	if (this->m_CellItems.count(cellId) && this->ref_scn->get()->getNetwork().getCellIds().count(cellId)) {
		//get all the things
		const DISCO2_API::Cell* thisCell = this->ref_scn->get()->getNetwork().getCell(cellId);
		const auto& visInfo = thisCell->getVisInfo();
		DCellItem* cellItem = this->m_CellItems.at(cellId);

		//update the size
		cellItem->updateSize(thisCell->getNumOfLanes(), thisCell->getLength());
		//only addd in the coord and orientation if visInfo has valid point
		if (visInfo.getPoints().size()) {
			const auto& visInfoPt = thisCell->getVisInfo().getPoint();
			//update the coord and orientation
			cellItem->setPos(this->ref_coordTransform->transformToDisplay(visInfoPt.m_X, visInfoPt.m_Y));
			cellItem->setRotation(visInfoPt.m_Orientation);
		}
		else {
			cellItem->setPos({0,0});
		}
		cellItem->update();
		emit itemsChanged();
	}
}

void DGraphicsScene::updateDArcItem(int arcId) {
	//make sure function no throw
	if (this->m_ArcItems.count(arcId) && this->ref_scn->get()->getNetwork().getArcIds().count(arcId)) {
		//get all the things
		const DISCO2_API::Arc* thisArc = this->ref_scn->get()->getNetwork().getArc(arcId);
		const auto& visInfo = thisArc->getVisInfo();
		DPolyArcItem* arcItem = this->m_ArcItems.at(arcId);

		//Update all the things
		//update the size
		arcItem->updateSize(thisArc->getVf() * this->ref_scn->get()->getNetwork().getSimTimeStepSize() / 1000, 
			thisArc->getNumOfLanes(), thisArc->getNumOfRows());
		if (visInfo.getPoints().size()) {
			//figure out the points
			QList<QPointF> pts;
			for (auto& p : visInfo.getPoints())
				pts.push_back(this->ref_coordTransform->transformToDisplay(p.m_X, p.m_Y));
			arcItem->updatePos(pts);

			//deal with the internal cells
			QList<QList<DCellItem*>> cellItems;
			for (auto& row : thisArc->getCellIds()) {
				QList<DCellItem*> tCellItems;
				for (auto& cellId : row) {
					tCellItems.push_back(this->getDCellItem(cellId));
					this->updateDCellItem(cellId);
				}
				cellItems.push_back(tCellItems);
			}
			arcItem->setCellItemRotPos(cellItems);
		}
		else {
			arcItem->setPos({ 0,0 });
		}
		arcItem->update();
		emit itemsChanged();
	}
}

void DGraphicsScene::updateDJctItem(int jctId) {
	//make sure function no throw
	if (this->m_JctItems.count(jctId) && this->ref_scn->get()->getNetwork().getJctIds().count(jctId)) {
		//get all the things
		const auto& visInfo = this->ref_scn->get()->getNetwork().getJctNode(jctId).getVisInfo();
		DJctItem* jctItem = this->m_JctItems.at(jctId);

		if (visInfo.getType() == DISCO2_API::VisInfo::Type_VisInfo::Point) {
			const auto& vp0 = visInfo.getPoint();
			jctItem->setPos(this->ref_coordTransform->transformToDisplay(vp0.m_X, vp0.m_Y));
		}
		else {
			jctItem->setPos({ 0,0 });
		}
		jctItem->update();
		emit itemsChanged();
	}
}

void DGraphicsScene::updateDmdSnkItem(int cellId) {
	//make sure function no throw
	if (this->m_DmdSnkCellItems.count(cellId) && this->ref_scn->get()->getNetwork().getDemandSinkCellIds().count(cellId)) {
		//get all the things
		const DISCO2_API::Cell_DemandSink* thisDmdSnkCell = this->ref_scn->get()->getNetwork().getCell_DemandSink(cellId);
		const auto& visInfo = thisDmdSnkCell->getVisInfo();
		DDemandSinkCellItem* DmdSnkCellItem = this->m_DmdSnkCellItems.at(cellId);


		//only addd in the coord and orientation if visInfo has valid point
		if (visInfo.getPoints().size()) {
			const auto& visInfoPt = thisDmdSnkCell->getVisInfo().getPoint();
			//update the coord and orientation
			DmdSnkCellItem->setPos(this->ref_coordTransform->transformToDisplay(visInfoPt.m_X, visInfoPt.m_Y));
			DmdSnkCellItem->setRotation(visInfoPt.m_Orientation);
			DmdSnkCellItem->setScale(visInfoPt.m_Scale);
		}
		else {
			DmdSnkCellItem->setPos({ 0,0 });
		}
		//update the size
		DmdSnkCellItem->updateSize(thisDmdSnkCell->getNumOfLanes());

		DmdSnkCellItem->update();
		emit itemsChanged();
	}
}

// --- --- --- --- --- Show/Hide Graphic Items --- --- --- --- ---

void DGraphicsScene::setShowAllDmdSnkCell(bool isShow) {
	for (auto& entry : this->m_DmdSnkCellItems)
		entry.second->setVisible(isShow);
}

void DGraphicsScene::setShowAllCell(bool isShow) {
	for (auto& entry : this->m_CellItems)
		entry.second->setVisible(isShow);
}

void DGraphicsScene::setShowAllArc(bool isShow) {
	for (auto& entry : this->m_ArcItems)
		entry.second->setVisible(isShow);
}

void DGraphicsScene::setShowAllJct(bool isShow) {
	for(auto& entry : this->m_JctItems)
		entry.second->setVisible(isShow);
}

void DGraphicsScene::setShowDetailJctLinkView(int jctId, bool isShowDetail) {
	if (this->m_PseudoJLinkItems.count(jctId))
		for(auto& jctLinkEntry : this->m_PseudoJLinkItems.at(jctId))
			jctLinkEntry.second->setVisible(!isShowDetail);

	if (this->m_JLinkItems.count(jctId))
		for (auto& jctLinkEntry : this->m_JLinkItems.at(jctId))
			jctLinkEntry.second->setVisible(isShowDetail);

	if (this->m_JCellItems.count(jctId))
		for (auto jctLinkEntry : this->m_JCellItems.at(jctId))
			for (auto cellItemEntry : jctLinkEntry.second)
				cellItemEntry.second->setVisible(isShowDetail);
}

void DGraphicsScene::setShowDetailJctLinkView(int jctId, int jctLinkId, bool isShowDetail) {
	if (this->m_PseudoJLinkItems.count(jctId) && this->m_PseudoJLinkItems.at(jctId).count(jctLinkId))
		this->m_PseudoJLinkItems.at(jctId).at(jctLinkId)->setVisible(!isShowDetail);

	if (this->m_JLinkItems.count(jctId) && this->m_JLinkItems.at(jctId).count(jctLinkId))
		this->m_JLinkItems.at(jctId).at(jctLinkId)->setVisible(isShowDetail);
	
	if (this->m_JCellItems.count(jctId) && this->m_JCellItems.at(jctId).count(jctLinkId))
		for (auto cellItemEntry : this->m_JCellItems.at(jctId).at(jctLinkId))
			cellItemEntry.second->setVisible(isShowDetail);
}

void DGraphicsScene::resetAllCellColor() {
	for (auto& cell : this->m_CellItems) {
		cell.second->setUseCustomColor(false);
		cell.second->update();
	}
	for (auto& cell : this->m_DmdSnkCellItems) {
		cell.second->setUseCustomColor(false);
		cell.second->update();
	}
}

void DGraphicsScene::resetAllCellText() {
	for (auto& cell : this->m_CellItems) {
		cell.second->setUseText(false);
		cell.second->update();
	}
	for (auto& cell : this->m_DmdSnkCellItems) {
		cell.second->setUseText(false);
		cell.second->update();
	}
}

// --- --- --- --- --- Private Utils --- --- --- --- ---

void DGraphicsScene::calArcCellCoords(int arcId) {
	//Make sure function no throw
	if (this->ref_scn->get()->getNetwork().getArcIds().count(arcId)) {
		//Get all the things
		const DISCO2_API::Arc* thisArc = this->ref_scn->get()->getNetwork().getArc(arcId);
		const auto& visInfo = thisArc->getVisInfo();
		//arc real pts
		const auto& pt0 = visInfo.getPoints().at(0);
		const auto& pt1 = visInfo.getPoints().at(1);

		//rotation angles of the cells
		double deltaX = pt1.m_X - pt0.m_X;
		double deltaY = pt1.m_Y - pt0.m_Y;
		double real_angle = std::atan2(deltaY, deltaX);	//anti-clockwise from x positive direction (radian)
		double cosB = std::cos(real_angle);
		double sinB = std::sin(real_angle);

		//arc stats
		double cellLength = thisArc->getVf() * this->ref_scn->get()->getNetwork().getSimTimeStepSize() / 1000;
		double arc_d = this->ref_coordTransform->calRealDistance(
			this->ref_coordTransform->makeQPointF(pt0), 
			this->ref_coordTransform->makeQPointF(pt1));
		int arc_rows = thisArc->getNumOfRows();
		int arc_lanes = thisArc->getNumOfLanes();

		//assign the centroid locations
		double thisD = arc_d - cellLength / 2;
		for (auto& row : thisArc->getCellIds()) {

			//keep track where the lane needs to be at
			int laneDone = 0;
			for (int& cellId : row) {
				//Get all the things
				const DISCO2_API::Cell* thisCell = this->ref_scn->get()->getNetwork().getCell(cellId);
				DISCO2_API::VisInfo out = thisCell->getVisInfo();
				int cell_lanes = thisCell->getNumOfLanes();
				
				//setup visInfo
				out.clearPoints();
				out.setType(DISCO2_API::VisInfo::Type_VisInfo::Point);

				//figure out the w location
				double thisW = -(0.5 * (cell_lanes - arc_lanes) + laneDone) * DGraphicsScene::laneWidth;

				//cal the real location
				//Formula for rotating a vector in 2D https://matthew-brett.github.io/teaching/rotation_2d.html
				double cellRealX = (cosB * thisD) - (sinB * thisW);
				double cellRealY = (sinB * thisD) + (cosB * thisW);

				//store the visInfo
				out.addPoint(DISCO2_API::VisInfo::VisInfo_Pt{ pt0.m_X + cellRealX, pt0.m_Y + cellRealY, qRadiansToDegrees(-real_angle) });
				this->ref_scn->get()->getNetEditor().cell_setVisInfo(cellId, std::move(out));
				//re-adjust the lanes done
				laneDone += cell_lanes;
			}
			//re-adjust the d location
			thisD -= cellLength;
		}
	}
}