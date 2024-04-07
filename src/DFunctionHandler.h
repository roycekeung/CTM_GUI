#pragma once

#include <QObject>

#include <memory>
#include <list>

class DMainWindow;
class DGraphicsView;
class DGraphicsScene;
class DDockWidgetHandler;
class DRunManager;
class DTabToolBar;
class I_Function;
class DBaseNaviFunction;
class DBaseMapFunction;
class DDefaultParamData;
class CoordTransform;
#include "Utils/DDefaultParamData.h"

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
 Manages the lifetime and relays graphics scene events to all active functions

 I_Functions must be passed into this Handler for initiation and gain access to other 
 internal handler classes
*/
class DFunctionHandler : public QObject {
	Q_OBJECT

private:
	DMainWindow* ref_mainWindow = nullptr;
	DGraphicsView* ref_graphicsView = nullptr;
	DGraphicsScene* ref_graphicsScene = nullptr;
	DDockWidgetHandler* ref_dockWidgetHandler = nullptr;
	DRunManager* ref_runManager = nullptr;
	DTabToolBar* ref_tabToolBar = nullptr;
	DBaseNaviFunction* ref_baseNaviFunction = nullptr;
	DBaseMapFunction* ref_baseMapFunction = nullptr;
	std::shared_ptr<DISCO2_API::Scenario>* ref_scn = nullptr;
	DefaultParamDataSet* ref_dParam = nullptr;
	CoordTransform* ref_coordTransform = nullptr;

	std::list<I_Function*> m_activeFunctions{};

	friend class DMainWindow;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DFunctionHandler();

	~DFunctionHandler();

	void setRef(DMainWindow* mainWindow, DGraphicsView* graphicsView, DGraphicsScene* graphicsScene,
		DDockWidgetHandler* dockWidgetHandler, DRunManager* runManager, DTabToolBar* tabToolBar, 
		std::shared_ptr<DISCO2_API::Scenario>* scn, 
		CoordTransform* coordTransform, DefaultParamDataSet* dparam);

	// --- --- --- --- --- Event Filter for Redistribute to Functions --- --- --- --- ---

	/**
	 Function will be init and managed by this class until close function called
	 Transfers the ownership of the function to this class, must call closeFunction to delete function
	*/
	void addFunction(I_Function* function);

	/**
	 Deletes and remove function from active function
	*/
	void closeFunction(I_Function* function);

	// --- --- --- --- --- Event Filter for Redistribute to Functions --- --- --- --- ---

	bool eventFilter(QObject* watched, QEvent* event) override;

};
