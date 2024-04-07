#include "DJctEditFunction.h"

#include "DFunctionHandler.h"
#include "Functions/DBaseNaviFunction.h"
#include "Functions/Junction/DJunctionModeFunction.h"
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

DJctEditFunction::DJctEditFunction(int jctId) : QObject(nullptr), I_Function(), m_jctId(jctId) {
}

DJctEditFunction::~DJctEditFunction() {
}

void DJctEditFunction::initFunctionHandler() {
	//setup the panel widget
	this->m_panelWidet = new DJctNodePanel{ DJctNodePanel::Type_JctPanel::edit, &(this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId)) };
	this->ref_dockWidgetHandler->setRightDockWidget(this->m_panelWidet, "Edit Junction");
	this->m_panelWidet->setupEditButtons(this);

	//cache the original pos
	this->ref_jctItem = this->ref_graphicsScene->getDJctItem(this->m_jctId);

	//set up the graphics scene
	this->ref_graphicsScene->clearSelection();

	//disable other stuff
	this->ref_baseNaviFunction->setIgnoreEvents(true);
	this->ref_tabToolBar->setEnabled(false);
}

// --- --- --- --- --- Checking and creation Functions --- --- --- --- ---

void DJctEditFunction::checkAndEditJct() {
	try {
		auto& netEditor = this->ref_scn->get()->getNetEditor();
		//create the jct
		netEditor.jct_setSignalised(this->m_jctId, this->m_panelWidet->getIsSigInput());
		netEditor.jct_setYellowBox(this->m_jctId, this->m_panelWidet->getIsBoxJctInput());

		//deal with visInfo
		DISCO2_API::VisInfo vis{};
		vis.setType(DISCO2_API::VisInfo::Type_VisInfo::Point);
		vis.addPoint(this->ref_coordTransform->makeVisInfoPt(
			this->ref_coordTransform->transformToReal(this->ref_jctItem->pos())));
		vis.setName(this->m_panelWidet->getNameInput().toStdString());
		vis.setDetails(this->m_panelWidet->getDetailsInput().toStdString());
		netEditor.jct_setVisInfo(this->m_jctId, std::move(vis));

		//close if succeeded
		this->cancel();
	}
	catch (std::exception& e) {
		//print fail msg in the widget
		this->m_panelWidet->failMessage({ e.what() });
	}
}

void DJctEditFunction::cancel() {
	//have graphics scene update / revert back to whatever is in core
	this->ref_graphicsScene->updateDJctItem(this->m_jctId);
	this->ref_graphicsScene->update();

	//remove the arc panel
	this->ref_dockWidgetHandler->removeAndDeleteRightDockWidget();

	//re-enable stuff
	this->ref_baseNaviFunction->setIgnoreEvents(false);
	this->ref_tabToolBar->setEnabled(true);

	// end this function
	this->closeFunction();
}

void DJctEditFunction::cancelAndJctMode() {
	//have graphics scene update / revert back to whatever is in core
	this->ref_graphicsScene->updateDJctItem(this->m_jctId);
	this->ref_graphicsScene->update();

	//remove the arc panel
	this->ref_dockWidgetHandler->removeAndDeleteRightDockWidget();

	//re-enable stuff
	this->ref_baseNaviFunction->setIgnoreEvents(false);
	this->ref_tabToolBar->setEnabled(true);

	//invoke jct mode function
	this->ref_baseNaviFunction->initJctModeFunction(this->m_jctId);

	// end this function
	this->closeFunction();
}

// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---

bool DJctEditFunction::keyPressEvent(QKeyEvent* keyEvent) {
	if (keyEvent->key() == Qt::Key::Key_Escape) {
		keyEvent->accept();

		if (this->m_dragging) {
			this->m_dragging = false;
			//reset jct node location
			this->ref_jctItem->setPos(this->t_dragStartPosition);
		}
		else
			this->cancel();

		return true;
	}

	return false;
}

bool DJctEditFunction::mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	if (this->m_dragging) {
		//only move when the drag distace is long enough
		if ((mouseEvent->pos() - this->t_dragStartPosition).manhattanLength() > QApplication::startDragDistance()) {
			//move the spine pt
			this->ref_jctItem->setPos(mouseEvent->scenePos());
			this->ref_jctItem->update();

			mouseEvent->accept();
			return true;
		}
	}
	return false;
}

bool DJctEditFunction::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	if (this->ref_graphicsScene->items(mouseEvent->scenePos()).count(this->ref_jctItem)) {
		this->m_dragging = true;
		this->t_dragStartPosition = this->ref_jctItem->pos();
		mouseEvent->accept();
		return true;
	}
	return false;
}

bool DJctEditFunction::mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	if (this->m_dragging) {
		//just have the jct be at the last place move was at for simplicity
		this->m_dragging = false;

		//allow save after move
		this->m_panelWidet->enableConfirmButton();

		mouseEvent->accept();
		return true;
	}
	return false;
}

