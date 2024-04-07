#include "DFunctionHandler.h"

#include "DMainWindow.h"
#include "DGraphicsView.h"
#include "DGraphicsScene.h"
#include "DDockWidgetHandler.h"
#include "DRunManager.h"
#include "DTabToolBar.h"
#include "Functions/I_Function.h"
#include "Functions/DBaseNaviFunction.h"
#include "Functions/DBaseMapFunction.h"

#include "qevent.h"
#include "qdebug.h"

#include <QGraphicsSceneEvent>
#include <QInputMethodEvent>

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DFunctionHandler::DFunctionHandler() : QObject() {

}

DFunctionHandler::~DFunctionHandler() {
	for (auto& f : this->m_activeFunctions)
		delete f;
}

void DFunctionHandler::setRef(DMainWindow* mainWindow, DGraphicsView* graphicsView, DGraphicsScene* graphicsScene, 
		DDockWidgetHandler* dockWidgetHandler, DRunManager* runManager, DTabToolBar* tabToolBar,
		std::shared_ptr<DISCO2_API::Scenario>* scn, 
		CoordTransform* coordTransform, DefaultParamDataSet* dparam) {
	
	this->setParent(mainWindow);
	this->ref_mainWindow = mainWindow;
	this->ref_graphicsView = graphicsView;
	this->ref_graphicsScene = graphicsScene;
	this->ref_dockWidgetHandler = dockWidgetHandler;
	this->ref_runManager = runManager;
	this->ref_tabToolBar = tabToolBar;
	this->ref_scn = scn;
	this->ref_coordTransform = coordTransform;
	this->ref_dParam = dparam;

	//Event Filter for interception
	this->ref_graphicsScene->installEventFilter(this);

	//init base function of things will work
	this->ref_baseNaviFunction = new DBaseNaviFunction{};
	this->addFunction(this->ref_baseNaviFunction);
	this->ref_baseMapFunction = new DBaseMapFunction{};
	this->addFunction(this->ref_baseMapFunction);
}

// --- --- --- --- --- Event Filter for Redistribute to Functions --- --- --- --- ---

void DFunctionHandler::addFunction(I_Function* function) {
	this->m_activeFunctions.emplace_back(function);
	function->setRefs(this->ref_mainWindow, this->ref_graphicsView, this->ref_graphicsScene,
		this->ref_dockWidgetHandler, this->ref_runManager, this->ref_tabToolBar, 
		this->ref_scn, this, this->ref_baseNaviFunction, this->ref_baseMapFunction, 
		this->ref_coordTransform, this->ref_dParam);
	function->initFunctionHandler();
}

void DFunctionHandler::closeFunction(I_Function* function) {
	delete function;	//this is soooo dangerous -JLo
	this->m_activeFunctions.remove(function); 
}

// --- --- --- --- --- Event Filter for Redistribute to Functions --- --- --- --- ---

bool DFunctionHandler::eventFilter(QObject* watched, QEvent* event) {
	if (watched == this->ref_graphicsScene) {
		switch (event->type()) {
		case QEvent::GraphicsSceneContextMenu:
			qDebug() << "GraphicsSceneContextMenu";
			for (auto& f : this->m_activeFunctions)
				if (f->contextMenuEvent(static_cast<QGraphicsSceneContextMenuEvent*>(event)))
					return true;
			break;

		case QEvent::GraphicsSceneDragEnter:
			qDebug() << "GraphicsSceneDragEnter";
			for (auto& f : this->m_activeFunctions)
				if (f->dragEnterEvent(static_cast<QGraphicsSceneDragDropEvent*>(event)))
					return true;
			break;

		case QEvent::GraphicsSceneDragLeave:
			qDebug() << "GraphicsSceneDragLeave";
			for (auto& f : this->m_activeFunctions)
				if (f->dragLeaveEvent(static_cast<QGraphicsSceneDragDropEvent*>(event)))
					return true;
			break;

		case QEvent::GraphicsSceneDragMove:
			qDebug() << "GraphicsSceneDragMove";
			for (auto& f : this->m_activeFunctions)
				if (f->dragMoveEvent(static_cast<QGraphicsSceneDragDropEvent*>(event)))
					return true;
			break;

		case QEvent::GraphicsSceneDrop:
			qDebug() << "GraphicsSceneDrop";
			for (auto& f : this->m_activeFunctions)
				if (f->dropEvent(static_cast<QGraphicsSceneDragDropEvent*>(event)))
					return true;
			break;

		case QEvent::GraphicsSceneHelp:
			qDebug() << "GraphicsSceneHelp";
			for (auto& f : this->m_activeFunctions)
				if (f->helpEvent(static_cast<QGraphicsSceneHelpEvent*>(event)))
					return true;
			break;

		case QEvent::InputMethod:
			qDebug() << "InputMethod";
			for (auto& f : this->m_activeFunctions)
				if (f->inputMethodEvent(static_cast<QInputMethodEvent*>(event)))
					return true;
			break;

		case QEvent::KeyPress:
			qDebug() << "KeyPress";
			for (auto& f : this->m_activeFunctions)
				if (f->keyPressEvent(static_cast<QKeyEvent*>(event)))
					return true;
			break;

		case QEvent::KeyRelease:
			qDebug() << "KeyRelease";
			for (auto& f : this->m_activeFunctions)
				if (f->keyReleaseEvent(static_cast<QKeyEvent*>(event)))
					return true;
			break;

		case QEvent::GraphicsSceneMouseDoubleClick:
			qDebug() << "GraphicsSceneMouseDoubleClick";
			for (auto& f : this->m_activeFunctions)
				if (f->mouseDoubleClickEvent(static_cast<QGraphicsSceneMouseEvent*>(event)))
					return true;
			break;

		case QEvent::GraphicsSceneMouseMove:
			qDebug() << "GraphicsSceneMouseMove";
			for (auto& f : this->m_activeFunctions)
				if (f->mouseMoveEvent(static_cast<QGraphicsSceneMouseEvent*>(event)))
					return true;
			break;

		case QEvent::GraphicsSceneMousePress:
			qDebug() << "GraphicsSceneMousePress";
			for (auto& f : this->m_activeFunctions)
				if (f->mousePressEvent(static_cast<QGraphicsSceneMouseEvent*>(event)))
					return true;
			break;

		case QEvent::GraphicsSceneMouseRelease:
			qDebug() << "GraphicsSceneMouseRelease";
			for (auto& f : this->m_activeFunctions)
				if (f->mouseReleaseEvent(static_cast<QGraphicsSceneMouseEvent*>(event)))
					return true;
			break;

		case QEvent::GraphicsSceneWheel:
			qDebug() << "GraphicsSceneWheel";
			for (auto& f : this->m_activeFunctions)
				if (f->wheelEvent(static_cast<QGraphicsSceneWheelEvent*>(event)))
					return true;
			break;

		default:
			// No appilcable event handler for this event
			break;
		}
	}
	return false;
}
