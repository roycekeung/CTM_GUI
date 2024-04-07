#pragma once

#include <memory>

#include <QMainWindow>
#include <QDockWidget>

#include <Scenario.h>

class DDockWidgetHandler;
class DGraphicsScene;
class DGraphicsView;
class DFunctionHandler;
class QToolBar;
class DTabToolBar;
class DRunManager;
class CoordTransform;
class DefaultParamDataSet;

class DMainWindow : public QMainWindow {
	Q_OBJECT

private:

	std::shared_ptr<DISCO2_API::Scenario> m_scn;		// !!! Do NOT use as a shared_ptr, other classes must only take a ref to this, so to allow scn change !!!

	DFunctionHandler* m_functionHandler;
	DDockWidgetHandler* m_dockWidgetHandler;
	CoordTransform* m_coordTransform;
	DGraphicsScene* m_scene;
	DGraphicsView* m_view;
	DRunManager* m_runManager;

	DefaultParamDataSet* m_dparam;

	QToolBar* m_toolBar;
	DTabToolBar* m_tabToolBar;

public:
	
	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DMainWindow(QWidget* parent = nullptr);

	~DMainWindow();

public slots:

	void newOnStartUp();

public slots:
	void closeEvent(QCloseEvent* event);
};
