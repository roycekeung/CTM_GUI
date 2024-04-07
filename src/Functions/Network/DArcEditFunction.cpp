#include "DArcEditFunction.h"

#include <unordered_set>

#include "DBaseNaviFunction.h"
#include "DTabToolBar.h"
#include "PanelWidgets/Network/DArcPanel.h"
#include "DDockWidgetHandler.h"
#include "GraphicsItems/DGraphicsScene.h"
#include "GraphicsItems/DGraphicsView.h"
#include "GraphicsItems/DCellConnectorItem.h"
#include "GraphicsItems/DArcSpinePointItem.h"
#include "GraphicsItems/DPolyArcItem.h"
#include "GraphicsItems/DCellItem.h"
#include "Utils/CoordTransform.h"
#include "Utils/DDefaultParamData.h"
#include "DBaseNaviFunction.h"

#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>

#include "Arc.h"
#include "Editor_Net.h"

#include <QDebug>

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DArcEditFunction::DArcEditFunction(int arcId) : QObject(nullptr), I_Function(), m_arcId(arcId) {
}

DArcEditFunction::~DArcEditFunction() {
}

void DArcEditFunction::initFunctionHandler() {
	//setup the panel widget
	this->m_panelWidet = new DArcPanel{ DArcPanel::Type_ArcPanel::edit, this->ref_scn->get()->getNetwork().getArc(this->m_arcId), this->ref_dParam };
	this->ref_dockWidgetHandler->setRightDockWidget(this->m_panelWidet, "Edit Arc");
	this->m_panelWidet->setEditable(true);
	this->m_panelWidet->setupEditButtons(this);

	const auto* thisArc = this->ref_scn->get()->getNetwork().getArc(this->m_arcId);
	this->m_orgVisInfo = thisArc->getVisInfo();
	//addin the spine points
	for (auto pt : this->m_orgVisInfo.getPoints()) {
		DArcSpinePointItem* tSpinePt = new DArcSpinePointItem{};
		this->ref_graphicsScene->addItem(tSpinePt);
		this->t_spinePts.push_back(tSpinePt);
		tSpinePt->setPos(this->ref_coordTransform->transformToDisplay(pt.m_X, pt.m_Y));
	}

	//make display clean
	this->ref_graphicsScene->removeAllDCellConnector_C2C();
	this->ref_graphicsScene->removeAllDCellConnector_C2J();

	//disable other stuff
	this->ref_baseNaviFunction->setIgnoreEvents(true);
	this->ref_tabToolBar->setEnabled(false);
}

// --- --- --- --- --- Checking and creation Functions --- --- --- --- ---

void DArcEditFunction::checkAndEditArc() {
	std::unordered_set<int> oldCellIds{};
	for (auto& row : this->ref_scn->get()->getNetwork().getArc(this->m_arcId)->getCellIds())
		for (auto& cellId : row)
			oldCellIds.insert(cellId);

	try {
		//update the arc
		this->ref_scn->get()->getNetEditor().arc_updateLength(this->m_arcId, this->m_panelWidet->getLengthInput());
		this->ref_scn->get()->getNetEditor().arc_updateParam(
			this->m_arcId, this->m_panelWidet->getQInput(), this->m_panelWidet->getKInput(),
			this->m_panelWidet->getVfInput(), this->m_panelWidet->getWInput()
		);
		//VisInfo - copied from "temp" to keep points
		DISCO2_API::VisInfo tVisInfo = this->ref_scn->get()->getNetwork().getArc(this->m_arcId)->getVisInfo();
		tVisInfo.setName(this->m_panelWidet->getNameInput().toStdString());
		tVisInfo.setDetails(this->m_panelWidet->getDetailsInput().toStdString());
		this->ref_scn->get()->getNetEditor().arc_setVisInfo(this->m_arcId, std::move(tVisInfo));
	}
	catch (std::exception& e) {
		//print fail msg in the widget
		this->m_panelWidet->failMessage({ e.what() });
		//early exit as operation failed
		return;
	}

	// save newly amended inputted params to default data struct bakcup
	this->ref_dParam->m_ArcFlowParams.NumberOfLane = this->ref_scn->get()->getNetwork().getArc(this->m_arcId)->getNumOfLanes();
	this->ref_dParam->m_ArcFlowParams.FreeFlowSpeed = this->ref_scn->get()->getNetwork().getArc(this->m_arcId)->getVf();
	this->ref_dParam->m_ArcFlowParams.BackwardShockwaveSpeed = this->ref_scn->get()->getNetwork().getArc(this->m_arcId)->getW();
	this->ref_dParam->m_ArcFlowParams.JamDensity = this->ref_scn->get()->getNetwork().getArc(this->m_arcId)->getK();
	this->ref_dParam->m_ArcFlowParams.SaturationFlow = this->ref_scn->get()->getNetwork().getArc(this->m_arcId)->getQ();

	std::unordered_set<int> newCellIds{};
	for (auto& row : this->ref_scn->get()->getNetwork().getArc(this->m_arcId)->getCellIds())
		for (auto& cellId : row)
			newCellIds.insert(cellId);
	//deal with the graphics items
	for (auto& oldCellId : oldCellIds)
		if (!newCellIds.count(oldCellId))
			this->ref_graphicsScene->removeDCellItem(oldCellId);
	for (auto& newCellId : newCellIds)
		if (!oldCellIds.count(newCellId))
			this->ref_graphicsScene->addDCellItem(newCellId);
	this->ref_graphicsScene->reloadLists();
	this->ref_graphicsScene->updateDArcItem(this->m_arcId);

	//re-enable baseNaviFunction
	this->ref_baseNaviFunction->setIgnoreEvents(false);

	//make selected so that it would show all the connecters within
	auto ptr = this->ref_graphicsScene->getDArcItem(this->m_arcId);
	if (ptr != nullptr)
		ptr->setSelected(true);
	this->ref_baseNaviFunction->showConnectorsOfSelected();

	//close if succeeded
	this->cleanup();
	this->closeFunction();
}

void DArcEditFunction::cleanup() {
	//delete the temp drag points
	for (auto spinePt : this->t_spinePts) {
		this->ref_graphicsScene->removeItem(spinePt);
		delete spinePt;
	}

	//remove the arc panel
	this->ref_dockWidgetHandler->removeAndDeleteRightDockWidget();

	//re-enable stuff
	this->ref_baseNaviFunction->setIgnoreEvents(false);
	this->ref_tabToolBar->setEnabled(true);
}

void DArcEditFunction::cancel() {
	//replace the original visInfo
	this->ref_scn->get()->getNetEditor().arc_setVisInfo(this->m_arcId, std::move(this->m_orgVisInfo));
	this->ref_graphicsScene->updateDArcItem(this->m_arcId);

	// end this function
	this->cleanup();
	this->closeFunction();
}

void DArcEditFunction::recalLength() {
	double out = 0;
	auto ptItr = this->t_spinePts.begin();
	QPointF prevPt = (*ptItr)->scenePos();
	while (++ptItr != this->t_spinePts.end()) {
		QPointF thisPt = (*ptItr)->scenePos();
		out += this->ref_coordTransform->calRealDistance(
			this->ref_coordTransform->transformToReal(prevPt),
			this->ref_coordTransform->transformToReal(thisPt)
		);
		prevPt = thisPt;
	}
	this->m_panelWidet->setLengthValue(out);
}

// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---

bool DArcEditFunction::keyPressEvent(QKeyEvent* keyEvent) {
	if (keyEvent->key() == Qt::Key::Key_Escape) {
		keyEvent->accept();

		if (this->m_dragging) {
			this->m_dragging = false;
			if (this->m_newSpinePt) {
				//delete mid point
				this->t_spinePts.remove(this->t_movingPt);
				this->ref_graphicsScene->removeItem(this->t_movingPt);
				delete this->t_movingPt;
			}
			else {
				//revert the moving point
				this->t_movingPt->setPos(this->t_dragStartPosition);
			}
			//reset the path
			this->updateTempArc();
		}
		else
			this->cancel();

		return true;
	}
	else if (keyEvent->key() == Qt::Key::Key_Delete) {
		if (this->t_movingPt->isSelected()) {
			//delete mid point
			this->t_spinePts.remove(this->t_movingPt);
			this->ref_graphicsScene->removeItem(this->t_movingPt);
			delete this->t_movingPt;
			//reset the path
			this->updateTempArc();
		}
		keyEvent->accept();
	}

	return false;
}

bool DArcEditFunction::mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	if (this->m_dragging) {
		//only move when the drag distace is long enough
		if ((mouseEvent->pos() - this->t_dragStartPosition).manhattanLength() > QApplication::startDragDistance()) {
			//move the spine pt
			this->t_movingPt->setPos(mouseEvent->scenePos());
			this->t_movingPt->update();

			//do the scene arc update
			this->updateTempArc();

			mouseEvent->accept();
			return true;
		}
	}
	return false;
}

bool DArcEditFunction::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	this->t_movingPt = nullptr;
	//find if clicking a point, and register the click
	for (auto item : this->ref_graphicsScene->items(mouseEvent->scenePos())) {
		switch (item->type()) {
		case (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::ArcSpinePt):
		{
			//find the location of the spine point
			bool found = false;
			int i = 0;
			for (auto spinePt : this->t_spinePts) {
				if (spinePt == item) {
					this->t_movingPt = spinePt;
					found = true;
					break;
				}
				i++;
			}

			//tidy up the stuff
			if (found) {
				this->m_dragging = true;
				this->t_dragStartPosition = this->t_movingPt->scenePos();
				this->t_movingPt->setPos(mouseEvent->scenePos());
				this->t_movingPt->setSelected(true);
				this->t_movingPt->update();
				mouseEvent->accept();
				return true;
			}
			break;
		}
		case (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::ArcItem):
			if( (dynamic_cast<DPolyArcItem*>(item))->getArcId() == this->m_arcId &&
				(mouseEvent->button() == Qt::MouseButton::RightButton && mouseEvent->modifiers().testFlag(Qt::KeyboardModifier::ControlModifier)) ) {
				this->m_dragging = true;
				this->m_newSpinePt = true;

				//find closest spine pt
				auto closestPtItr = this->t_spinePts.begin();
				int closestIndex = 0;
				std::vector<double> disList{};
				double closestLength = DBL_MAX;
				int i = 0;
				for (auto sPtItr = this->t_spinePts.begin(); sPtItr != this->t_spinePts.end(); sPtItr++) {
					double thisDist = (mouseEvent->scenePos() - (*sPtItr)->scenePos()).manhattanLength();
					disList.push_back(thisDist);
					if (thisDist < closestLength) {
						closestIndex = i;
						closestLength = thisDist;
						closestPtItr = sPtItr;
					}
					i++;
				}
				//move the itr iff closestIndex is first or dist is shorter towards the back of list
				if (closestIndex == 0 || 
					((closestIndex < this->t_spinePts.size() - 1) && (disList.at(closestIndex - 1) > disList.at(closestIndex + 1))))
					closestPtItr++;
			
				//add temp spine pt
				this->t_movingPt = new DArcSpinePointItem{};
				this->ref_graphicsScene->addItem(this->t_movingPt);
				this->t_movingPt->setPos(mouseEvent->scenePos());
				this->t_dragStartPosition = mouseEvent->scenePos();
				this->t_spinePts.insert(closestPtItr, this->t_movingPt);

				//do the scene arc update
				this->updateTempArc();

				mouseEvent->accept();
				return true;
			}
			break;
		}
	}
	return false;
}

bool DArcEditFunction::mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	if (this->m_dragging) {
		//just have the arc be at the last place move was at for simplicity
		this->m_dragging = false;
		this->m_newSpinePt = false;

		//move the spine pt
		this->t_movingPt->setPos(mouseEvent->scenePos());
		this->t_movingPt->update();

		//do the scene arc update
		this->updateTempArc();
		mouseEvent->accept();

		//allow confirm edit
		this->m_panelWidet->enableConfirmButton();

		return true;
	}
	return false;
}

// --- --- --- --- --- Private Utils --- --- --- --- ---

void DArcEditFunction::updateTempArc() {
	//move the arc
	DISCO2_API::VisInfo tempVisInfo = this->m_orgVisInfo;
	tempVisInfo.clearPoints();
	for (auto tSpinePt : this->t_spinePts) {
		tempVisInfo.addPoint(this->ref_coordTransform->makeVisInfoPt(
			this->ref_coordTransform->transformToReal(tSpinePt->scenePos())));
	}

	//do the scene arc update
	this->ref_scn->get()->getNetEditor().arc_setVisInfo(this->m_arcId, std::move(tempVisInfo));
	this->ref_graphicsScene->updateDArcItem(this->m_arcId);
}
