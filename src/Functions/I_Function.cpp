#include "I_Function.h"

#include "DFunctionHandler.h"


// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

I_Function::I_Function() {}

I_Function::~I_Function() {}

void I_Function::setRefs(DMainWindow* mainWindow, DGraphicsView* graphicsView, DGraphicsScene* graphicsScene,
	DDockWidgetHandler* dockWidgetHandler, DRunManager* runManager, DTabToolBar* tabToolBar, 
	std::shared_ptr<DISCO2_API::Scenario>* scn,
	DFunctionHandler* functionHandler, DBaseNaviFunction* baseNaviFunction, DBaseMapFunction* baseMapFunction,
	CoordTransform* coordTransform, DefaultParamDataSet* dparam) {

	this->ref_mainWindow = mainWindow;
	this->ref_graphicsView = graphicsView;
	this->ref_graphicsScene = graphicsScene;
	this->ref_dockWidgetHandler = dockWidgetHandler;
	this->ref_runManager = runManager;
	this->ref_tabToolBar = tabToolBar;
	this->ref_scn = scn;
	this->ref_functionHandler = functionHandler;
	this->ref_baseNaviFunction = baseNaviFunction;
	this->ref_baseMapFunction = baseMapFunction;
	this->ref_coordTransform = coordTransform;
	this->ref_dParam = dparam;
}

void I_Function::closeFunction() {
	this->ref_functionHandler->closeFunction(this);
}
