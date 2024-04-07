#pragma once

#include <memory>
#include <Scenario.h>

class DMainWindow;
class DGraphicsView;
class DGraphicsScene;
class DDockWidgetHandler;
class DRunManager;
class DTabToolBar;
class DFunctionHandler;
class DBaseNaviFunction;
class DBaseMapFunction;
struct DefaultParamDataSet;
class CoordTransform;

class QGraphicsSceneContextMenuEvent;
class QGraphicsSceneDragDropEvent;
class QGraphicsSceneHelpEvent;
class QInputMethodEvent;
class QKeyEvent;
class QGraphicsSceneMouseEvent;
class QGraphicsSceneWheelEvent;

namespace DISCO2_API {
class Scenario;
}

/**
 * Interface for all standalone function features
 * Create standard way to interact with graphics scene/view events and functions
 * Also auto-loads useful pointers
 */
class I_Function {

protected:

	DFunctionHandler* ref_functionHandler = nullptr;
	DMainWindow* ref_mainWindow = nullptr;
	DGraphicsView* ref_graphicsView = nullptr;
	DGraphicsScene* ref_graphicsScene = nullptr;
	DDockWidgetHandler* ref_dockWidgetHandler = nullptr;
	DRunManager* ref_runManager = nullptr;
	DTabToolBar* ref_tabToolBar = nullptr;
	DBaseNaviFunction* ref_baseNaviFunction = nullptr;
	DBaseMapFunction* ref_baseMapFunction = nullptr;
	std::shared_ptr<DISCO2_API::Scenario>* ref_scn = nullptr;
	CoordTransform* ref_coordTransform = nullptr;
	DefaultParamDataSet* ref_dParam = nullptr;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	I_Function();

	virtual ~I_Function();

	void setRefs(DMainWindow* mainWindow, DGraphicsView* graphicsView, DGraphicsScene* graphicsScene, 
		DDockWidgetHandler* dockWidgetHandler, DRunManager* runManager, DTabToolBar* tabToolBar,
		std::shared_ptr<DISCO2_API::Scenario>* scn,
		DFunctionHandler* functionHandler, DBaseNaviFunction* baseNaviFunction, DBaseMapFunction* baseMapFunction,
		CoordTransform* coordTransform, DefaultParamDataSet* dparam);

	/**
	 This function gets called when setRefs is completed, and the ref pointers are safe to call
	 Implementations should init their widget at this point
	*/
	virtual void initFunctionHandler() = 0;

	/**
	 This function trigger the deletion of this obj, 
	 WARNING potentially unsafe if used incorrectly
	 must be the last thing to be executed
	*/
	void closeFunction();

public:

	// --- --- --- --- --- Re-implementable functions for intercepting DGraphicsScene events --- --- --- --- ---

	/**
	 Only reimplement event handlers that are required
	 Implementation should return true iff no other objects should receive the event anymore
	 Otherwise return false to allow propagation to other functions or to scene/graphics item level
	*/

	virtual bool contextMenuEvent(QGraphicsSceneContextMenuEvent* contextMenuEvent) { return false; }
	virtual bool dragEnterEvent(QGraphicsSceneDragDropEvent* event) { return false; }
	virtual bool dragLeaveEvent(QGraphicsSceneDragDropEvent* event) { return false; }
	virtual bool dragMoveEvent(QGraphicsSceneDragDropEvent* event) { return false; }
	virtual bool dropEvent(QGraphicsSceneDragDropEvent* event) { return false; }
	virtual bool helpEvent(QGraphicsSceneHelpEvent* helpEvent) { return false; }
	virtual bool inputMethodEvent(QInputMethodEvent* event) { return false; }
	virtual bool keyPressEvent(QKeyEvent* keyEvent) { return false; }
	virtual bool keyReleaseEvent(QKeyEvent* keyEvent) { return false; }
	virtual bool mouseDoubleClickEvent(QGraphicsSceneMouseEvent* mouseEvent) { return false; }
	virtual bool mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) { return false; }
	virtual bool mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) { return false; }
	virtual bool mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent) { return false; }
	virtual bool wheelEvent(QGraphicsSceneWheelEvent* wheelEvent) { return false; }

};
