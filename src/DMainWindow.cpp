#include "DMainWindow.h"

#include <QToolBar>
#include <QCloseEvent>

#include "qdebug.h"

#include "DDockWidgetHandler.h"
#include "DGraphicsScene.h"
#include "DGraphicsView.h"
#include "DFunctionHandler.h"
#include "DTabToolBar.h"
#include "DRunManager.h"
#include "Utils/CoordTransform.h"
#include "Utils/DDefaultParamData.h"

#include "Functions/File/DLoadScenFunc.h"
#include "Functions/File/DNewScenFunc.h"
#include "Functions/Home/DZoomToExtentFunction.h"
#include "Functions/DBaseMapFunction.h"
#include "Utils/TileGenerator_GeoInfoMap_HK1980.h"

DMainWindow::DMainWindow(QWidget* parent) : QMainWindow(parent), 
	m_functionHandler(new DFunctionHandler{}),
	m_scn(DISCO2_API::Scenario::createNewScenario()),
	m_dockWidgetHandler(new DDockWidgetHandler{ this }),
	m_coordTransform(new CoordTransform{ CoordTransform::CoordSystem::HK1980 }),
	m_scene(new DGraphicsScene{ this, &(this->m_scn), this->m_coordTransform }),
	m_view(new DGraphicsView{ this->m_scene, this }),
	m_runManager(new DRunManager{}),
	m_toolBar(new QToolBar{ this }),
	m_tabToolBar(new DTabToolBar{ this->m_toolBar, this->m_functionHandler }),
	m_dparam(new DefaultParamDataSet{}) {

	this->setAttribute(Qt::WA_DeleteOnClose);

	//#init funciton handler
	this->m_functionHandler->setRef(this, this->m_view, this->m_scene, this->m_dockWidgetHandler, 
		this->m_runManager, this->m_tabToolBar, &(this->m_scn), this->m_coordTransform, this->m_dparam);

	//# do the event filter
	this->m_scene->installEventFilter(this->m_functionHandler);

	//#	handle window sizes
	this->resize(900, 600);		//pre-set a small window size
	this->showMaximized();		//make the app fullscreen

	//#	init tool bar
	this->addToolBar(Qt::ToolBarArea::TopToolBarArea, this->m_toolBar);
	this->m_toolBar->addWidget(this->m_tabToolBar);
	this->m_toolBar->setFloatable(false);
	this->m_toolBar->setMovable(false);
	this->m_toolBar->show();
	this->m_tabToolBar->show();

	//#	init network display
	this->setCentralWidget(this->m_view);

}

DMainWindow::~DMainWindow() {
	delete this->m_functionHandler;
	delete this->m_coordTransform;
	delete this->m_runManager;
	delete this->m_dparam;
}

void DMainWindow::newOnStartUp() {
	//free to trigger stuff on start up

#ifdef _DEBUG
	//DEBUG mode hard coded load scn
	this->m_functionHandler->addFunction(new DLoadScenFunc{ "testNet.xml" });
	this->m_scn->loadSignalSet("testSig.xml");
	this->m_scn->loadDemandSet("testDmd.xml");

#else
	//trigger ask for new scn
	this->m_functionHandler->addFunction(new DNewScenFunc{});
#endif 
}


void DMainWindow::closeEvent(QCloseEvent* event) {
	//! Ignore the event by default.. otherwise the window will be closed always.
	event->ignore();

	event->accept(); // Do not need to save nothing... accept the event and close the app

}
