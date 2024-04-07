#include "DShowHideFunction.h"

// DISCO2_GUI lib
#include "../GraphicsItems/DGraphicsScene.h"
#include "../GraphicsItems/DGraphicsView.h"
#include "../DDockWidgetHandler.h"
#include "../DBaseNaviFunction.h"
#include "../PanelWidgets/Home/DShowHidePanel.h"

// Qt lib
#include <QKeyEvent>
#include <QPushButton>


// --- --- --- --- --- Check initial State --- --- --- --- ---
void DShowHideFunction::SetInitialCheckState() {
	// check of the initial show/hide state of all graphical items; set them back to the corresponding checkox
	this->m_ShowHidePanelWidet->getHideDmdSinkCellsCheckBox()->setChecked(!this->ref_baseNaviFunction->getIsShowAllDmdSnkCells());
	this->m_ShowHidePanelWidet->getHideCellsCheckBox()->setChecked(!this->ref_baseNaviFunction->getIsShowAllCells());
	this->m_ShowHidePanelWidet->getHideArcsCheckBox()->setChecked(!this->ref_baseNaviFunction->getIsShowAllArcs());
	this->m_ShowHidePanelWidet->getHideJctsCheckBox()->setChecked(!this->ref_baseNaviFunction->getIsShowAllJcts());
}


// --- --- --- --- --- Constructor Destructor --- --- --- --- ---
DShowHideFunction::DShowHideFunction(): QObject(nullptr){

}

DShowHideFunction::~DShowHideFunction(){
	// re-able the baseNaviFunction
	this->ref_baseNaviFunction->setIgnoreEvents(false);
}


// --- --- --- --- --- override initFunctionHandler from I_Function --- --- --- --- ---
void DShowHideFunction::initFunctionHandler() {
	//set up the graphics scene
	this->ref_graphicsScene->clearSelection();

	this->m_ShowHidePanelWidet = new DShowHidePanel();
	this->ref_dockWidgetHandler->setRightDockWidget(this->m_ShowHidePanelWidet, "Create Demand/ Sink Cell");

	// connect the buttons to the actual function in order to activate the actions
	this->m_ShowHidePanelWidet->ConnectButtons(this);
	this->m_ShowHidePanelWidet->ConnectCheckBoxes(this->ref_baseNaviFunction);

	// check of the initial show/hide state of all graphical items; set them back to the corresponding checkox
	SetInitialCheckState();

	// ignore the baseNaviFunction
	this->ref_baseNaviFunction->setIgnoreEvents(true);

}

// --- --- --- --- --- Close Functions --- --- --- --- ---
void DShowHideFunction::cancel() {

	//remove the demand sink cell panel
	this->ref_dockWidgetHandler->removeAndDeleteRightDockWidget();

	// clear selection after finish creating or not yet created
	this->ref_graphicsScene->clearSelection();

	// destroy function
	this->closeFunction();
}

// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---
bool DShowHideFunction::keyPressEvent(QKeyEvent* keyEvent) {
	if (keyEvent->key() == Qt::Key::Key_Escape) {
		keyEvent->accept();
		// end of this function
		cancel();
		return true;
	}

	// continue to handle other events
	return false;
}
