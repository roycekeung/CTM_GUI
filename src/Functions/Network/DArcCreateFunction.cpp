#include "DArcCreateFunction.h"

#include "Functions/DBaseNaviFunction.h"
#include "PanelWidgets/Network/DArcPanel.h"
#include "DDockWidgetHandler.h"
#include "DTabToolBar.h"
#include "GraphicsItems/DGraphicsScene.h"
#include "GraphicsItems/DCellConnectorItem.h"
#include "GraphicsItems/DArcSpinePointItem.h"
#include "Utils/CoordTransform.h"
#include "Utils/DDefaultParamData.h"

#include <QGraphicsPathItem>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>

#include "Arc.h"
#include "VisInfo.h"
#include "Editor_Net.h"

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DArcCreateFunction::DArcCreateFunction() : QObject(nullptr), I_Function() {
}

DArcCreateFunction::~DArcCreateFunction() {
}

void DArcCreateFunction::initFunctionHandler() {
	//setup the panel widget
	this->m_panelWidet = new DArcPanel{ DArcPanel::Type_ArcPanel::create, nullptr, this->ref_dParam };
	this->ref_dockWidgetHandler->setRightDockWidget(this->m_panelWidet, "Create Arc");
	this->m_panelWidet->setEditable(false);
	this->m_panelWidet->setupCreateButtons(this);

	//set up the graphics scene
	this->ref_graphicsScene->clearSelection();

	//disable other stuff
	this->ref_baseNaviFunction->setIgnoreEvents(true);
	this->ref_tabToolBar->setEnabled(false);
}

// --- --- --- --- --- Checking and creation Functions --- --- --- --- ---

void DArcCreateFunction::checkAndCreateArc() {
	try {
		//create the arc
		int newArcId = this->ref_scn->get()->getNetEditor().arc_create(
			this->m_panelWidet->getLengthInput(), this->m_panelWidet->getQInput(), this->m_panelWidet->getKInput(),
			this->m_panelWidet->getVfInput(), this->m_panelWidet->getWInput(), this->m_panelWidet->getNumOfLanesInput()
		);

		// save newly amended inputted params to default data struct bakcup
		this->ref_dParam->m_ArcFlowParams.NumberOfLane = this->m_panelWidet->getNumOfLanesInput();
		this->ref_dParam->m_ArcFlowParams.FreeFlowSpeed = this->m_panelWidet->getVfInput();
		this->ref_dParam->m_ArcFlowParams.BackwardShockwaveSpeed = this->m_panelWidet->getWInput();
		this->ref_dParam->m_ArcFlowParams.JamDensity = this->m_panelWidet->getKInput();
		this->ref_dParam->m_ArcFlowParams.SaturationFlow = this->m_panelWidet->getQInput();

		//deal with visInfo
		DISCO2_API::VisInfo vis{};
		vis.setType(DISCO2_API::VisInfo::Type_VisInfo::Vector);
		for (auto* spinePt : this->t_spinePts)
			vis.addPoint(this->ref_coordTransform->makeVisInfoPt(
				this->ref_coordTransform->transformToReal(spinePt->pos())));
		vis.setName(this->m_panelWidet->getNameInput().toStdString());
		vis.setDetails(this->m_panelWidet->getDetailsInput().toStdString());
		this->ref_scn->get()->getNetEditor().arc_setVisInfo(newArcId, std::move(vis));

		//have graphics scene make it
		this->ref_graphicsScene->addDArcItem(newArcId);
		this->ref_graphicsScene->update();
		//close if succeeded
		this->cancel();
	}
	catch (std::exception& e) {
		//print fail msg in the widget
		this->m_panelWidet->failMessage({ e.what() });
	}
}

void DArcCreateFunction::cancel() {
	//delete all the graphical things
	if (this->m_clickStage != ClickStage::firstClick) {
		//delete the temp cell connector standing in as pseudo arc
		this->ref_graphicsScene->removeItem(this->t_line);
		delete this->t_line;
		this->ref_graphicsScene->removeItem(this->t_psuedoArc);
		delete this->t_psuedoArc;
		//delete the temp drag points
		for (auto spinePt : this->t_spinePts) {
			this->ref_graphicsScene->removeItem(spinePt);
			delete spinePt;
		}
	}

	//remove the arc panel
	this->ref_dockWidgetHandler->removeAndDeleteRightDockWidget();
	//re-enable stuff
	this->ref_baseNaviFunction->setIgnoreEvents(false);
	this->ref_tabToolBar->setEnabled(true);

	// end this function
	this->closeFunction();
}

void DArcCreateFunction::recalLength() {
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

bool DArcCreateFunction::keyPressEvent(QKeyEvent* keyEvent) {
	if (keyEvent->key() == Qt::Key::Key_Escape) {
		keyEvent->accept();

		switch (this->m_clickStage) {
		case editDrag:
			this->m_clickStage = createdFirstArc;
			//reset the moving pt
			this->t_movingPt->setPos(this->t_dragStartPosition);
			//reset the path
			this->resetPath();
			this->recalLength();
			break;
		case addSpine:
			this->m_clickStage = createdFirstArc;
			//remove the temp spinePt
			this->t_spinePts.remove(this->t_movingPt);
			this->ref_graphicsScene->removeItem(this->t_movingPt);
			delete this->t_movingPt;
			this->t_movingPt = nullptr;
			//reset the path
			this->resetPath();
			this->recalLength();
			break;
		default:
			this->cancel();
			break;
		}

		return true;
	}
	else if (keyEvent->key() == Qt::Key::Key_Delete) {
		if (this->t_movingPt->isSelected()) {
			//delete mid point
			this->t_spinePts.remove(this->t_movingPt);
			this->ref_graphicsScene->removeItem(this->t_movingPt);
			delete this->t_movingPt;
			this->t_movingPt = nullptr;
			//reset the path
			this->resetPath();
			this->recalLength();
		}
		keyEvent->accept();
	}

	return false;
}

bool DArcCreateFunction::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	if (mouseEvent->button() == Qt::MouseButton::LeftButton &&
		this->m_clickStage == ClickStage::createDrag) {

		//workaround to solve mouse release then double click
		this->ref_graphicsScene->removeItem(this->t_movingPt);
		this->t_spinePts.remove(this->t_movingPt);
		delete this->t_movingPt;
		//update the arrow line
		this->resetPath();
		this->recalLength();
		//change state
		this->m_clickStage = createdFirstArc;
		//allow editing
		this->m_panelWidet->setEditable(true);
		this->m_panelWidet->enableConfirmButton();

		mouseEvent->accept();
		return true;
	}
	return false;
}

bool DArcCreateFunction::mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	switch (this->m_clickStage) {
	case createDrag:
	{
		//move the spine pt
		this->t_movingPt->setPos(mouseEvent->scenePos());
		this->t_movingPt->update();
		//set the line
		this->t_psuedoArc->setEndPt(mouseEvent->scenePos());
		//length
		this->recalLength();
		mouseEvent->accept();
		return true;
	}
	case editDrag:
	case addSpine:
	{
		//only move when the drag distace is long enough
		if ((mouseEvent->pos() - this->t_dragStartPosition).manhattanLength() > QApplication::startDragDistance()) {
			//move the spine pt
			this->t_movingPt->setPos(mouseEvent->scenePos());
			this->t_movingPt->update();
			//update the path
			this->resetPath();
			//length
			this->recalLength();
			mouseEvent->accept();
			return true;
		}
	}
	}
	return false;
}

bool DArcCreateFunction::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	if(mouseEvent->button() == Qt::MouseButton::LeftButton)
		this->ref_graphicsScene->clearSelection();
		switch (this->m_clickStage) {

		case createDrag:	//creating extra spine points
		{
			this->t_movingPt->setVisible(true);
			//make a spine point
			this->t_movingPt = new DArcSpinePointItem{};
			this->t_movingPt->setVisible(false);
			this->t_spinePts.push_back(this->t_movingPt);
			this->ref_graphicsScene->addItem(this->t_movingPt);
			this->t_movingPt->setPos(mouseEvent->scenePos());
			this->t_movingPt->update();
			//update the path
			this->resetPath();

			mouseEvent->accept();
			break;
		}

		default:
			break;

		case firstClick:		//waitng for first click
		{
			//make a spine point
			DArcSpinePointItem* tSpine = new DArcSpinePointItem{};
			this->t_spinePts.push_back(tSpine);
			this->ref_graphicsScene->addItem(tSpine);
			tSpine->setPos(mouseEvent->scenePos());
			tSpine->update();
			//make the line
			this->t_line = new QGraphicsPathItem{};
			this->t_line->setZValue(QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::CellConnectorItem);
			QPainterPath path{ mouseEvent->scenePos() };
			this->t_line->setPath(path);
			this->t_line->setPen(QPen{ Qt::black, 0.5, Qt::SolidLine, Qt::FlatCap, Qt::PenJoinStyle::MiterJoin });
			this->ref_graphicsScene->addItem(this->t_line);
			this->t_line->update();
			//make the line with arrow
			this->t_psuedoArc = new DCellConnectorItem{ mouseEvent->scenePos(), mouseEvent->scenePos() };
			this->ref_graphicsScene->addItem(this->t_psuedoArc);
			this->t_psuedoArc->update();
			//make a spine point
			this->t_movingPt = new DArcSpinePointItem{};
			this->t_movingPt->setVisible(false);
			this->t_spinePts.push_back(this->t_movingPt);
			this->ref_graphicsScene->addItem(this->t_movingPt);
			this->t_movingPt->setPos(mouseEvent->scenePos());
			this->t_movingPt->update();
			//change state
			this->m_clickStage = createDrag;
			break;
		}

		case createdFirstArc:	//allow fine tuning coord
		{
			this->t_movingPt = nullptr;
			//find if clicking a point, and register the click
			for (auto item : this->ref_graphicsScene->items(mouseEvent->scenePos())) {
				if (item->type() == QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::ArcSpinePt) {
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
						this->m_clickStage = editDrag;
						this->t_dragStartPosition = this->t_movingPt->scenePos();
						this->t_movingPt->setPos(mouseEvent->scenePos());
						this->t_movingPt->setSelected(true);
						this->t_movingPt->update();
						break;
					}
				}
				else if (item == this->t_line || item == t_psuedoArc) {
					if (mouseEvent->button() == Qt::MouseButton::RightButton && mouseEvent->modifiers().testFlag(Qt::KeyboardModifier::ControlModifier)) {
						this->m_clickStage = addSpine;

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
						this->resetPath();
						break;
					}
				}
			}
			break;
		}

		}

	//intercepts all scene event so notthing else gets selected
	mouseEvent->accept();
	return true;
}

bool DArcCreateFunction::mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	switch (this->m_clickStage) {

	case ClickStage::editDrag:
	case ClickStage::addSpine:
	{
		this->recalLength();
		this->m_clickStage = createdFirstArc;
		mouseEvent->accept();
		break;
	}
	}
	return false;
}

// --- --- --- --- --- Private Utils --- --- --- --- ---

void DArcCreateFunction::resetPath() {
	//move the arrow line
	this->t_psuedoArc->setEndPt(this->t_spinePts.back()->scenePos());
	this->t_psuedoArc->setStartPt((*(++this->t_spinePts.rbegin()))->scenePos());
	//update the path
	QPainterPath path{ this->t_spinePts.front()->scenePos() };
	auto itr = ++this->t_spinePts.begin();
	for (int i = 0; i < this->t_spinePts.size() - 2; i++) {
		path.lineTo((*itr)->scenePos());
		itr++;
	}
	this->t_line->setPath(path);
}