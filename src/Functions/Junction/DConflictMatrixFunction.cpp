#include "DConflictMatrixFunction.h"

#include "PanelWidgets/Junction/DConflictMatrixPanel.h"
#include "Utils/DConflictMatrixItem.h"
#include "Functions/DBaseNaviFunction.h"
#include "Functions/Junction/DJunctionModeFunction.h"
#include "GraphicsItems/DGraphicsScene.h"
#include "GraphicsItems/DPseudoJctLinkItem.h"
#include "DTabToolBar.h"

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DConflictMatrixFunction::DConflictMatrixFunction() : QObject(nullptr), I_Function() {
}

DConflictMatrixFunction::~DConflictMatrixFunction() {
}

void DConflictMatrixFunction::initFunctionHandler() {
	this->m_jctId = this->ref_baseNaviFunction->getJctModeFunction()->getCurrentJctId();
	auto& jctNode = this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId);
	this->ref_graphicsScene->setShowDetailJctLinkView(this->m_jctId, false);
	this->ref_graphicsScene->clearSelection();

	//base settings
	this->ref_baseNaviFunction->getJctModeFunction()->setIgnoreEvents(true);
	this->ref_tabToolBar->setEnabled(false);

	this->m_panel = new DConflictMatrixPanel{ jctNode.getHasSig() };
	this->m_panel->setupConnections(this);
	QObject::connect(this->m_panel, &QDialog::finished, this, &DConflictMatrixFunction::close);
	QObject::connect(this->ref_tabToolBar->getExitJctButton(), &QPushButton::clicked, this->m_panel, &QDialog::done);
	
	//grab current data
	auto& jctIds = jctNode.getJctLinkIds();
	this->m_jLinkIds = { jctIds.begin(), jctIds.end() };
	std::sort(this->m_jLinkIds.begin(), this->m_jLinkIds.end());
	auto& sgIds = jctNode.getSigGpIds();
	this->m_sigGpIds = { sgIds.begin(), sgIds.end() };
	std::sort(this->m_sigGpIds.begin(), this->m_sigGpIds.end());

	//setup the tables
	this->m_panel->setJctLinkIds(this->m_jLinkIds);
	this->m_panel->setSigGpIds(this->m_sigGpIds);
	this->calDefaultTable();
	this->loadCustomSettings();
	this->resetJLCellColors();
	this->resetSGCellColors();

	this->m_panel->exec();
}

// --- --- --- --- --- Slots for Panel --- --- --- --- ---

void DConflictMatrixFunction::close() {
	//reset color
	for (auto& entry : this->ref_graphicsScene->getPseudoJctLinksInJct(this->m_jctId)) {
		entry.second->setUseCustomColor(false);
		entry.second->update();
	}

	//base settings
	this->ref_baseNaviFunction->getJctModeFunction()->setIgnoreEvents(false);
	this->ref_tabToolBar->setEnabled(true);

	this->closeFunction();
}

void DConflictMatrixFunction::clickedConflict() {
	//get current state
	auto index = this->m_panel->getCurrentSelectedJLCell();
	auto type = this->m_panel->getJLItemDelegate()->getType(index.row(), index.column());
	int jLink1 = this->m_jLinkIds.at(index.row());
	int jLink2 = this->m_jLinkIds.at(index.column());
	//esc iff on the diagonal
	if (jLink1 == jLink2)
		return;

	//check default 
	//type noConflict = 0 -> false
	bool sameAsDefault = ((bool)type == this->m_defaultTable.at(index.row()).at(index.column()));
	
	//revert or add custom to visInfo
	DISCO2_API::VisInfo visInfo = this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId).getVisInfo();
	visInfo.removeSecret(this->keyString(jLink1, jLink2));
	visInfo.removeSecret(this->keyString(jLink2, jLink1));
	if (sameAsDefault) {
		visInfo.addSecret(this->keyString(jLink1, jLink2), type ? "False" : "True");
	}
	this->ref_scn->get()->getNetEditor().jct_setVisInfo(this->m_jctId, std::move(visInfo));

	//remove yielding if needed
	if (type) {
		auto& jct = this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId);
		auto& netEditor = this->ref_scn->get()->getNetEditor();
		std::unordered_set<int> jL1YieldTo = (jct.getIsYieldToLinkIdsSet(jLink1) ? jct.getYieldToLinkIds(jLink1) : std::unordered_set<int>{});
		std::unordered_set<int> jL2YieldTo = (jct.getIsYieldToLinkIdsSet(jLink2) ? jct.getYieldToLinkIds(jLink2) : std::unordered_set<int>{});
		jL1YieldTo.erase(jLink2);
		jL2YieldTo.erase(jLink1);
		netEditor.jctLink_setYieldToLinks(this->m_jctId, jLink1, std::move(jL1YieldTo));
		netEditor.jctLink_setYieldToLinks(this->m_jctId, jLink2, std::move(jL2YieldTo));
	}

	//reload everything
	this->loadCustomSettings();
	this->resetJLCellColors();
	this->resetSGCellColors();
	this->m_panel->updateTables();
	this->hightlightJL();
	this->m_panel->jLinkTableSelectionChanged();
}

void DConflictMatrixFunction::clickedPriority() {
	//get current state
	auto index = this->m_panel->getCurrentSelectedJLCell();
	auto type = this->m_panel->getJLItemDelegate()->getType(index.row(), index.column());
	int jLinkId1 = this->m_jLinkIds.at(index.row());
	int jLinkId2 = this->m_jLinkIds.at(index.column());
	//esc iff on the diagonal
	if (jLinkId1 == jLinkId2)
		return;

	//get everything
	auto& jct = this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId);
	auto& netEditor = this->ref_scn->get()->getNetEditor();
	std::unordered_set<int> jL1YieldTo = (jct.getIsYieldToLinkIdsSet(jLinkId1) ? jct.getYieldToLinkIds(jLinkId1) : std::unordered_set<int>{});
	std::unordered_set<int> jL2YieldTo = (jct.getIsYieldToLinkIdsSet(jLinkId2) ? jct.getYieldToLinkIds(jLinkId2) : std::unordered_set<int>{});
	jL1YieldTo.erase(jLinkId2);
	jL2YieldTo.erase(jLinkId1);

	//jam to core
	switch (type) {
	case DConflictMatrixItem::conflict:
	case DConflictMatrixItem::priorityLeft:
		//set priority to top
		jL1YieldTo.insert(jLinkId2);
		break;
	case DConflictMatrixItem::priorityTop:
		//set priority to left
		jL2YieldTo.insert(jLinkId1);
		break;
	}
	netEditor.jctLink_setYieldToLinks(this->m_jctId, jLinkId1, std::move(jL1YieldTo));
	netEditor.jctLink_setYieldToLinks(this->m_jctId, jLinkId2, std::move(jL2YieldTo));

	//reload everything
	this->resetJLCellColors();
	this->resetSGCellColors();
	this->m_panel->updateTables();
	this->hightlightJL();
}

void DConflictMatrixFunction::clickedRevertAll() {
	//remove all custom
	auto visInfo = this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId).getVisInfo();
	for (auto& id1 : this->m_jLinkIds)
		for (auto& id2 : this->m_jLinkIds)
			visInfo.removeSecret(this->keyString(id1, id2));
	this->ref_scn->get()->getNetEditor().jct_setVisInfo(this->m_jctId, std::move(visInfo));

	//remove all core yeilding
	for (auto& id : this->m_jLinkIds)
		this->ref_scn->get()->getNetEditor().jctLink_setIsYielding(this->m_jctId, id, false);

	//reload everything
	this->loadCustomSettings();
	this->resetJLCellColors();
	this->resetSGCellColors();
	this->m_panel->updateTables();
	this->hightlightJL();
}

void DConflictMatrixFunction::hightlightJL() {
	//grab all the things
	auto& jLinkItems = this->ref_graphicsScene->getPseudoJctLinksInJct(this->m_jctId);
	auto index = this->m_panel->getCurrentSelectedJLCell();
	int jLinkTop = this->m_jLinkIds.at(index.column());
	int jLinkLeft = this->m_jLinkIds.at(index.row());

	//reset color
	for (auto& entry : jLinkItems)
		entry.second->setUseCustomColor(false);
	
	//do the color
	switch (this->m_panel->getJLItemDelegate()->getType(index.row(), index.column())) {
	case DConflictMatrixItem::ConflictType::noConflict:
		if (jLinkItems.count(jLinkTop))
			jLinkItems.at(jLinkTop)->setUseCustomColor(true, DConflictMatrixItem::refColor_noConflict);
		if (jLinkItems.count(jLinkLeft))
			jLinkItems.at(jLinkLeft)->setUseCustomColor(true, DConflictMatrixItem::refColor_noConflict);
		break;
	case DConflictMatrixItem::ConflictType::conflict:
		if (jLinkItems.count(jLinkTop))
			jLinkItems.at(jLinkTop)->setUseCustomColor(true, DConflictMatrixItem::refColor_conflict);
		if (jLinkItems.count(jLinkLeft))
			jLinkItems.at(jLinkLeft)->setUseCustomColor(true, DConflictMatrixItem::refColor_conflict);
		break;
	case DConflictMatrixItem::ConflictType::priorityTop:
		if (jLinkItems.count(jLinkTop))
			jLinkItems.at(jLinkTop)->setUseCustomColor(true, DConflictMatrixItem::refColor_noConflict);
		if (jLinkItems.count(jLinkLeft))
			jLinkItems.at(jLinkLeft)->setUseCustomColor(true, DConflictMatrixItem::refColor_priorityConfigured);
		break;
	case DConflictMatrixItem::ConflictType::priorityLeft:
		if (jLinkItems.count(jLinkTop))
			jLinkItems.at(jLinkTop)->setUseCustomColor(true, DConflictMatrixItem::refColor_priorityConfigured);
		if (jLinkItems.count(jLinkLeft))
			jLinkItems.at(jLinkLeft)->setUseCustomColor(true, DConflictMatrixItem::refColor_noConflict);
		break;
	}

	//update
	for (auto& entry : jLinkItems)
		entry.second->update();
}

void DConflictMatrixFunction::hightlightSG() {
	//grab all the things
	auto& jLinkItems = this->ref_graphicsScene->getPseudoJctLinksInJct(this->m_jctId);
	auto index = this->m_panel->getCurrentSelectedSGCell();
	auto type = this->m_panel->getSGItemDelegate()->getType(index.row(), index.column());
	auto& topSigGpJLs = this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId).getSigGpJLinkIds(this->m_sigGpIds.at(index.column()));
	auto& leftSigGpJLs = this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId).getSigGpJLinkIds(this->m_sigGpIds.at(index.row()));

	//reset color
	for (auto& entry : jLinkItems)
		entry.second->setUseCustomColor(false);

	//set all color green first
	for (auto& id : topSigGpJLs)
		if (jLinkItems.count(id))
			jLinkItems.at(id)->setUseCustomColor(true, DConflictMatrixItem::refColor_noConflict);
	for (auto& id : leftSigGpJLs)
		if (jLinkItems.count(id))
			jLinkItems.at(id)->setUseCustomColor(true, DConflictMatrixItem::refColor_noConflict);

	//save the red JLs for ref
	std::unordered_set<int> redJL;

	if (type != DConflictMatrixItem::ConflictType::noConflict) {
		//override which ever are conflicted
		DConflictMatrixItem::ConflictType type = DConflictMatrixItem::ConflictType::noConflict; 
		auto* itemDelegate = this->m_panel->getJLItemDelegate();
		//check conflict
		for (auto& jLId1 : topSigGpJLs) {
			for (auto& jLId2 : leftSigGpJLs) {
				//find pos in vector
				int jLPos1 = -1;
				int jLPos2 = -1;
				int k = 0;
				for (auto& t : this->m_jLinkIds) {
					if (t == jLId1)
						jLPos1 = k;
					if (t == jLId2)
						jLPos2 = k;
					k++;
				}

				switch (itemDelegate->getType(jLPos2, jLPos1)) {
				case DConflictMatrixItem::ConflictType::conflict:
					redJL.insert(jLId1);
					redJL.insert(jLId2);
					if (jLinkItems.count(jLId1))
						jLinkItems.at(jLId1)->setUseCustomColor(true, DConflictMatrixItem::refColor_conflict);
					if (jLinkItems.count(jLId2))
						jLinkItems.at(jLId2)->setUseCustomColor(true, DConflictMatrixItem::refColor_conflict);
					break;
				case DConflictMatrixItem::ConflictType::priorityTop:
					if (!redJL.count(jLId2) && jLinkItems.count(jLId2))
						jLinkItems.at(jLId2)->setUseCustomColor(true, DConflictMatrixItem::refColor_priorityConfigured);
					break;
				case DConflictMatrixItem::ConflictType::priorityLeft:
					if (!redJL.count(jLId1) && jLinkItems.count(jLId1))
						jLinkItems.at(jLId1)->setUseCustomColor(true, DConflictMatrixItem::refColor_priorityConfigured);
					break;
				}
			}
			//escape the double loop
			if (type)
				break;
		}
	}

	//update
	for (auto& entry : jLinkItems)
		entry.second->update();
}

// --- --- --- --- --- Private Utils --- --- --- --- ---

std::string DConflictMatrixFunction::keyString(int id1, int id2) {
	return ("OverrideHasConflict:" + std::to_string(id1) + "+" + std::to_string(id2));
}

void DConflictMatrixFunction::calDefaultTable() {
	auto& jLinkItems = this->ref_graphicsScene->getPseudoJctLinksInJct(this->m_jctId);
	auto& jct = this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId);

	this->m_defaultTable.clear();
	for (auto& id1 : this->m_jLinkIds) {
		std::vector<bool> temp;
		for (auto& id2 : this->m_jLinkIds) {
			bool sameItem = (id1 == id2);
			bool overlap = jLinkItems.at(id1)->collidesWithItem(jLinkItems.at(id2));
			bool sameStart = (jct.getJctLink(id1)->getFromCellId() == jct.getJctLink(id2)->getFromCellId());

			temp.emplace_back(!sameItem && (overlap && !sameStart));
		}
		this->m_defaultTable.emplace_back(std::move(temp));
	}
}

void DConflictMatrixFunction::loadCustomSettings() {
	this->m_overrideConflict.clear();
	const auto& visInfo = this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId).getVisInfo();

	for (auto& id1 : this->m_jLinkIds) {
		for (auto& id2 : this->m_jLinkIds) {
			std::string value = visInfo.findSecret(this->keyString(id1, id2));
			if (!value.empty()) {
				this->m_overrideConflict.emplace_back(id1, id2, (value == "True"));
			}
		}
	}
}

void DConflictMatrixFunction::resetJLCellColors() {
	auto* itemDelegate = this->m_panel->getJLItemDelegate();
	auto& jct = this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId);

	for (int i = 0; i < this->m_jLinkIds.size(); i++) {
		int jLinkId1 = this->m_jLinkIds.at(i);
		std::unordered_set<int> jL1YieldTo = (jct.getIsYieldToLinkIdsSet(jLinkId1) ? jct.getYieldToLinkIds(jLinkId1) : std::unordered_set<int>{});
		for (int j = 0; j < this->m_jLinkIds.size(); j++) {
			int jLinkId2 = this->m_jLinkIds.at(j);
			DConflictMatrixItem::ConflictType type = DConflictMatrixItem::ConflictType::noConflict;

			//check default
			if (this->m_defaultTable.at(i).at(j))
				type = DConflictMatrixItem::ConflictType::conflict;

			//check overrides
			for (auto& entry : this->m_overrideConflict) {
				int a = std::get<0>(entry);
				int b = std::get<1>(entry);
				if ((a == jLinkId1 && b == jLinkId2) || (a == jLinkId2 && b == jLinkId1))
					type = (std::get<2>(entry) ? DConflictMatrixItem::ConflictType::conflict : DConflictMatrixItem::ConflictType::noConflict);
			}

			//check yielding
			if (type) {
				std::unordered_set<int> jL2YieldTo = (jct.getIsYieldToLinkIdsSet(jLinkId2) ? jct.getYieldToLinkIds(jLinkId2) : std::unordered_set<int>{});

				if (jL1YieldTo.count(jLinkId2))
					type = DConflictMatrixItem::ConflictType::priorityTop;
				else if (jL2YieldTo.count(jLinkId1))
					type = DConflictMatrixItem::ConflictType::priorityLeft;
			}

			itemDelegate->setType(i, j, type);
		}
	}
}

void DConflictMatrixFunction::resetSGCellColors() {
	auto* jlItemDelegate = this->m_panel->getJLItemDelegate();
	auto* sgItemDelegate = this->m_panel->getSGItemDelegate();
	auto& jct = this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId);

	for (int i = 0; i < this->m_sigGpIds.size(); i++) {
		auto& sigGpJLIds1 = jct.getSigGpJLinkIds(this->m_sigGpIds.at(i));

		for (int j = 0; j < this->m_sigGpIds.size(); j++) {
			auto& sigGpJLIds2 = jct.getSigGpJLinkIds(this->m_sigGpIds.at(j));

			DConflictMatrixItem::ConflictType type = DConflictMatrixItem::ConflictType::noConflict;
			//check conflict
			for (auto& jLId1 : sigGpJLIds1) {
				for (auto& jLId2 : sigGpJLIds2) {
					//find pos in vector
					int jLPos1 = -1;
					int jLPos2 = -1;
					int k = 0;
					for (auto& t : this->m_jLinkIds) {
						if (t == jLId1)
							jLPos1 = k;
						if (t == jLId2)
							jLPos2 = k;
						k++;
					}

					//figure out the color
					switch (jlItemDelegate->getType(jLPos1, jLPos2)) {
					case DConflictMatrixItem::ConflictType::conflict:
						type = DConflictMatrixItem::ConflictType::conflict;
						break;
					case DConflictMatrixItem::ConflictType::priorityLeft:
					case DConflictMatrixItem::ConflictType::priorityTop:
						type = DConflictMatrixItem::ConflictType::priorityConfigured;
						break;
					}
					//escape the double loop
					if (type == DConflictMatrixItem::ConflictType::conflict)
						break;
				}
				//escape the double loop
				if (type == DConflictMatrixItem::ConflictType::conflict)
					break;
			}

			sgItemDelegate->setType(i, j, type);
		}
	}
}
