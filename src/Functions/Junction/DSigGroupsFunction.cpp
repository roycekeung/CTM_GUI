#include "DSigGroupsFunction.h"

#include "PanelWidgets/Junction/DSigGroupsPanel.h"
#include "Functions/DBaseNaviFunction.h"
#include "Functions/Junction/DJunctionModeFunction.h"
#include "DDockWidgetHandler.h"
#include "DTabToolBar.h"
#include "GraphicsItems/DGraphicsScene.h"
#include "GraphicsItems/DPseudoJctLinkItem.h"

#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QSpinBox>

#include <Jct_Node.h>
#include <Editor_Net.h>

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DSigGroupsFunction::DSigGroupsFunction() : QObject(nullptr) {}

DSigGroupsFunction::~DSigGroupsFunction() {}

void DSigGroupsFunction::initFunctionHandler() {
	//set up the graphics scene
	this->ref_baseNaviFunction->getJctModeFunction()->setIgnoreEvents(true);
	this->m_jctId = this->ref_baseNaviFunction->getJctModeFunction()->getCurrentJctId();
	this->ref_graphicsScene->setShowDetailJctLinkView(this->m_jctId, false);
	this->ref_graphicsScene->clearSelection();

	//setup the panel widget
	this->m_panelWidet = new DSigGroupsPanel{ 
		this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId), 
		std::unordered_map<int, DPseudoJctLinkItem*>{this->ref_graphicsScene->getPseudoJctLinksInJct(this->m_jctId)} };
	this->ref_dockWidgetHandler->setRightDockWidget(this->m_panelWidet, "Signal Groups");
	this->m_panelWidet->setupConnections(this);

	//lock up tab tool bar
	this->ref_tabToolBar->setEnabled(false);
}

// --- --- --- --- --- Checking and creation Functions --- --- --- --- ---

void DSigGroupsFunction::addNewSigGp() {
	try {
		//make it in core
		int newSigGpId = this->ref_scn->get()->getSigEditor().sigGp_createAndAddJctLinks(this->m_jctId, {});
		//show in panel
		this->m_panelWidet->addSigGpToTable(newSigGpId);
	}
	catch (std::exception& e) {
		//print fail msg in the widget
		this->m_panelWidet->failMessage({ e.what() });
	}
}

void DSigGroupsFunction::editSigGp(int sigGpId) {
	if (this->m_selectMode) {
		//figure out which JLinks are selected
		std::unordered_map<DPseudoJctLinkItem*, int> tPsJLinkMap{};
		std::unordered_set<int> t_out{};
		for (auto& entry : this->ref_graphicsScene->getPseudoJctLinksInJct(this->m_jctId))
			tPsJLinkMap.emplace(entry.second, entry.first);
		for (auto& item : this->ref_graphicsScene->selectedItems())
			if (tPsJLinkMap.count(dynamic_cast<DPseudoJctLinkItem*>(item)))
				t_out.insert(tPsJLinkMap.at(dynamic_cast<DPseudoJctLinkItem*>(item)));

		//cause I didn't make a direct set func, so delete and remake sigGp
		if (this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId).getSigGpIds().count(sigGpId))
			this->ref_scn->get()->getSigEditor().sigGp_delete(this->m_jctId, sigGpId);

		try {
			//make it in core
			int newGpId = this->ref_scn->get()->getSigEditor().sigGp_createAndAddJctLinks(this->m_jctId, std::move(t_out), sigGpId);
			assert (newGpId == sigGpId);
		}
		catch (std::exception& e) {
			//print fail msg in the widget
			this->m_panelWidet->failMessage({ e.what() });
		}

		//manage the panel
		this->m_panelWidet->updateSigGpTable(sigGpId);
		this->m_panelWidet->completedEditSigGp();
		this->m_selectMode = false;
	}
	else {
		this->m_selectMode = true;
		this->t_currSigGp = sigGpId;
		//highlight the in gp JLinks
		auto& pJLinks = this->ref_graphicsScene->getPseudoJctLinksInJct(this->m_jctId);
		if (this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId).getSigGpIds().count(sigGpId))
			for (auto& entry : this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId).getSigGpJLinkIds(sigGpId))
				if (pJLinks.count(entry))
					pJLinks.at(entry)->setSelected(true);
	}
}

void DSigGroupsFunction::deleteSigGp(int sigGpId) {
	try {
		//make it in core
		this->ref_scn->get()->getSigEditor().sigGp_delete(this->m_jctId, sigGpId);
	}
	catch (std::exception& e) {
		//print fail msg in the widget
		this->m_panelWidet->failMessage({ e.what() });
	}
	//remove in panel
	//outside as only fail is that sigGp doesn't exist and so should be removed anyway -JLo
	this->m_panelWidet->removeSigGpFromTable(sigGpId);
}

void DSigGroupsFunction::cancel() {
	// save the min green edit
	for (int i = 0; i < this->m_panelWidet->getTableWdiget()->rowCount(); ++i) {
		int sigGpId = ((QSpinBox*)(this->m_panelWidet->getTableWdiget()->cellWidget(i, 0)))->value();
		this->ref_scn->get()->getSigEditor().sigGp_setMinGreen(
			this->m_jctId, 
			sigGpId, 
			((QSpinBox*)(this->m_panelWidet->getTableWdiget()->cellWidget(i, 3)))->value());
	}

	//revert the graphics scene
	this->ref_baseNaviFunction->getJctModeFunction()->setIgnoreEvents(false);
	this->ref_graphicsScene->setShowDetailJctLinkView(this->m_jctId, false);
	this->ref_graphicsScene->clearSelection();

	//remove the panel widget
	this->ref_dockWidgetHandler->removeAndDeleteRightDockWidget();

	//unlock tab tool bar
	this->ref_tabToolBar->setEnabled(true);

	this->closeFunction();
}

// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---

bool DSigGroupsFunction::keyPressEvent(QKeyEvent* keyEvent) {
	if (keyEvent->key() == Qt::Key::Key_Escape) {
		if (m_selectMode) {
			keyEvent->accept();
			//exit the edit
			this->editSigGp(this->t_currSigGp);
		}
		else {
			keyEvent->accept();
			this->cancel();
		}
		return true;
	}
	return false;
}

bool DSigGroupsFunction::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	if (this->m_selectMode)
		for (auto* topItem : this->ref_graphicsScene->items(mouseEvent->scenePos())) {
			//loop so that it can find the JLink
			if (topItem->type() == QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::PseudoJLinkItem) {
				//flip selection
				topItem->setSelected(!topItem->isSelected());
			}
		}

	//intercept all mouse click to disallow selecting other stuff
	return true;
}
