#include "DJctLinkEditFunction.h"

#include "Functions/DBaseNaviFunction.h"
#include "Functions/Junction/DJunctionModeFunction.h"
#include "PanelWidgets/Network/DArcPanel.h"
#include "DDockWidgetHandler.h"
#include "DTabToolBar.h"
#include "GraphicsItems/DGraphicsScene.h"
#include "GraphicsItems/DPseudoJctLinkItem.h"
#include "Utils/DDefaultParamData.h"

#include "Jct_Link.h"
#include "VisInfo.h"
#include "Editor_Net.h"

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DJctLinkEditFunction::DJctLinkEditFunction(int jctId, int jctLinkId) : 
	QObject(nullptr), m_jctId(jctId), m_jctLinkId(jctLinkId) {
}

DJctLinkEditFunction::~DJctLinkEditFunction() {
	
}

void DJctLinkEditFunction::initFunctionHandler() {
	//setup the panel widget
	this->m_panelWidet = new DArcPanel{ DArcPanel::Type_ArcPanel::editJL, 
		this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId).getJctLink(this->m_jctLinkId), 
		this->ref_dParam };
	this->ref_dockWidgetHandler->setRightDockWidget(this->m_panelWidet, "Edit Jct Link");
	this->m_panelWidet->setupEditJLButtons(this);

	//disable the baseNaviFunction
	this->ref_graphicsScene->clearSelection();
	this->ref_baseNaviFunction->getJctModeFunction()->setIgnoreEvents(true);

	//lock up tab tool bar
	this->ref_tabToolBar->setEnabled(false);
}

// --- --- --- --- --- Checking and creation Functions --- --- --- --- ---

void DJctLinkEditFunction::checkAndEditJctLink() {
	auto jctLink = this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId).getJctLink(this->m_jctLinkId);
	try {
		//update the cell
		this->ref_scn->get()->getNetEditor().jctLink_updateParam(
			this->m_jctId, this->m_jctLinkId,
			this->m_panelWidet->getQInput(), this->m_panelWidet->getKInput(),
			this->m_panelWidet->getVfInput(), this->m_panelWidet->getWInput(),
			this->m_panelWidet->getLengthInput()
		);

		// save newly amended inputted params to default data struct bakcup
		this->ref_dParam->m_JLinkFlowParams.NumberOfLane = this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId).getJctLink(this->m_jctLinkId)->getNumOfLanes();
		this->ref_dParam->m_JLinkFlowParams.FreeFlowSpeed = this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId).getJctLink(this->m_jctLinkId)->getVf();
		this->ref_dParam->m_JLinkFlowParams.BackwardShockwaveSpeed = this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId).getJctLink(this->m_jctLinkId)->getW();
		this->ref_dParam->m_JLinkFlowParams.JamDensity = this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId).getJctLink(this->m_jctLinkId)->getK();
		this->ref_dParam->m_JLinkFlowParams.SaturationFlow = this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId).getJctLink(this->m_jctLinkId)->getQ();


		//VisInfo
		DISCO2_API::VisInfo tVisInfo = jctLink->getVisInfo();
		tVisInfo.setName(this->m_panelWidet->getNameInput().toStdString());
		tVisInfo.setDetails(this->m_panelWidet->getDetailsInput().toStdString());
		this->ref_scn->get()->getNetEditor().arc_setVisInfo(jctLink->getId(), std::move(tVisInfo));
	}
	catch (std::exception& e) {
		//print fail msg in the widget
		this->m_panelWidet->failMessage({ e.what() });
		//early exit as operation failed
		return;
	}

	//update graphics
	this->ref_graphicsScene->removeAllJLinkItemInJct(this->m_jctId);
	this->ref_graphicsScene->addAllJLinkItemInJct(this->m_jctId);
	this->ref_graphicsScene->setShowDetailJctLinkView(this->m_jctId, this->m_jctLinkId);
	// end this function
	this->cancel();
}

void DJctLinkEditFunction::cancel() {
	//remove the arc panel
	this->ref_dockWidgetHandler->removeAndDeleteRightDockWidget();

	//re-enable JctModeFunction
	this->ref_baseNaviFunction->getJctModeFunction()->setIgnoreEvents(false);
	this->ref_graphicsScene->setShowDetailJctLinkView(this->m_jctId, false);
	//reenable the tabToolBar
	this->ref_tabToolBar->setEnabled(true);

	// end this function
	this->closeFunction();
}

void DJctLinkEditFunction::recalLength() {
	this->m_panelWidet->setLengthValue(this->ref_graphicsScene->getPseudoJctLinksInJct(this->m_jctId).at(this->m_jctLinkId)->getLength());
}
