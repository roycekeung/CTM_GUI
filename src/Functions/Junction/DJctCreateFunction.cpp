#include "DJctCreateFunction.h"

#include "Functions/DBaseNaviFunction.h"
#include "PanelWidgets/Junction/DJctNodePanel.h"
#include "DDockWidgetHandler.h"
#include "DTabToolBar.h"
#include "GraphicsItems/DGraphicsScene.h"
#include "GraphicsItems/DJctItem.h"
#include "Utils/CoordTransform.h"

#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>

#include "Jct_Node.h"
#include "VisInfo.h"
#include "Editor_Net.h"

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DJctCreateFunction::DJctCreateFunction() : QObject(nullptr), I_Function() {
}

DJctCreateFunction::~DJctCreateFunction() {
}

void DJctCreateFunction::initFunctionHandler() {
	//setup the panel widget
	this->m_panelWidet = new DJctNodePanel{ DJctNodePanel::Type_JctPanel::create };
	this->ref_dockWidgetHandler->setRightDockWidget(this->m_panelWidet, "Create Junction");
	this->m_panelWidet->setupCreateButtons(this);

	//set up the graphics scene
	this->ref_graphicsScene->clearSelection();

	//disable other stuff
	this->ref_baseNaviFunction->setIgnoreEvents(true);
	this->ref_tabToolBar->setEnabled(false);
}

// --- --- --- --- --- Checking and creation Functions --- --- --- --- ---

void DJctCreateFunction::checkAndCreateJct() {
#ifdef LOCKED_GUI_DEPLOY
	//create jct limit 3
	if (this->ref_scn->get()->getNetwork().getJctIds().size() >= 3) {
		this->m_panelWidet->failMessage("Not allowed to create more than 3 junctions");
		return;
	}
#endif

	try {
		auto& netEditor = this->ref_scn->get()->getNetEditor();
		//create the jct
		int newJctId = netEditor.jct_createJunctNode();
		netEditor.jct_setSignalised(newJctId, this->m_panelWidet->getIsSigInput());
		netEditor.jct_setYellowBox(newJctId, this->m_panelWidet->getIsBoxJctInput());

		//deal with visInfo
		DISCO2_API::VisInfo vis{};
		vis.setType(DISCO2_API::VisInfo::Type_VisInfo::Point);
		vis.addPoint(this->ref_coordTransform->makeVisInfoPt(
			this->ref_coordTransform->transformToReal(this->t_jctItem->pos())));
		vis.setName(this->m_panelWidet->getNameInput().toStdString());
		vis.setDetails(this->m_panelWidet->getDetailsInput().toStdString());
		netEditor.jct_setVisInfo(newJctId, std::move(vis));

		//have graphics scene make it
		this->ref_graphicsScene->addDJctItem(newJctId);
		this->ref_graphicsScene->update();
		//close if succeeded
		this->cancel();
	}
	catch (std::exception& e) {
		//print fail msg in the widget
		this->m_panelWidet->failMessage({ e.what() });
	}
}

void DJctCreateFunction::cancel() {
	//delete all the graphical things
	if (this->m_clickStage != ClickStage::firstClick) {
		//delete the temp jct item
		this->ref_graphicsScene->removeItem(this->t_jctItem);
		delete this->t_jctItem;
	}

	//remove the jct panel
	this->ref_dockWidgetHandler->removeAndDeleteRightDockWidget();
	//re-enable stuff
	this->ref_baseNaviFunction->setIgnoreEvents(false);
	this->ref_tabToolBar->setEnabled(true);

	// end this function
	this->closeFunction();
}

// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---

bool DJctCreateFunction::keyPressEvent(QKeyEvent* keyEvent) {
	if (keyEvent->key() == Qt::Key::Key_Escape) {
		keyEvent->accept();

		if (this->m_clickStage == editDrag)
			this->m_clickStage = createdFirstNode;
		else
			this->cancel();

		return true;
	}

	return false;
}

bool DJctCreateFunction::mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	switch (this->m_clickStage) {
	case editDrag:
		//only move when the drag distace is long enough
		if ((mouseEvent->pos() - this->t_dragStartPosition).manhattanLength() > QApplication::startDragDistance()) {
			//move the spine pt
			this->t_jctItem->setPos(mouseEvent->scenePos());
			this->t_jctItem->update();

			mouseEvent->accept();
			return true;
		}
	}
	return false;
}

bool DJctCreateFunction::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	if (mouseEvent->button() == Qt::MouseButton::LeftButton)
		switch (this->m_clickStage) {
		case firstClick:		//waitng for first click
		{
			//make a spine point
			this->t_jctItem = new DJctItem{};
			this->ref_graphicsScene->addItem(this->t_jctItem);
			this->t_jctItem->setPos(mouseEvent->scenePos());
			this->t_jctItem->update();
			//change state
			this->m_clickStage = createdFirstNode;
			//allow input
			this->m_panelWidet->setEditable(true);
			this->m_panelWidet->enableConfirmButton();
			break;
		}
		case createdFirstNode:	//allow fine tuning coor
		{
			//find if clicking a point, then register the click
			if (this->ref_graphicsScene->items(mouseEvent->scenePos()).count(this->t_jctItem)) {
				this->t_dragStartPosition = this->t_jctItem->pos();
				this->m_clickStage = editDrag;
			}
			break;
		}
		}

	//intercepts all scene event so notthing else gets selected
	mouseEvent->accept();
	return true;
}

bool DJctCreateFunction::mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	if (this->m_clickStage == ClickStage::editDrag) {
		this->m_clickStage = createdFirstNode;
		mouseEvent->accept();
	}
	return false;
}
