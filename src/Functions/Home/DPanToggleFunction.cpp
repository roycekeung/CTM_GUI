#include "DPanToggleFunction.h"

// Qt lib
#include <QKeyEvent>
#include <QPushButton>

// DISCO2_GUI lib
#include "../GraphicsItems/DGraphicsView.h"
#include "../DBaseNaviFunction.h"

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---
DPanToggleFunction::DPanToggleFunction(QPushButton* ToggleButton):m_ToggleBtn(ToggleButton), QObject(nullptr) {

}

DPanToggleFunction::~DPanToggleFunction(){

}

// --- --- --- --- --- override initFunctionHandler from I_Function --- --- --- --- ---
void DPanToggleFunction::initFunctionHandler() {
	// save the previous drag mode
	this->prev_dragMode = this->ref_graphicsView->dragMode();
	// set to pan mode
	this->ref_graphicsView->setDragMode(QGraphicsView::DragMode::ScrollHandDrag);

	//// ignore the baseNaviFunction
	//this->ref_baseNaviFunction->setIgnoreEvents(true);

	//Home Tab
	QObject::connect(this->m_ToggleBtn, &QPushButton::toggled, this, &DPanToggleFunction::cancel);

}

// --- --- --- --- --- Close Functions --- --- --- --- ---
void DPanToggleFunction::cancel(bool checked) {
	if (checked) {
		// continue
	}
	else {
		// set the pan mode back to previous set up 
		this->ref_graphicsView->setDragMode(this->prev_dragMode);

		//// re-able the baseNaviFunction
		//this->ref_baseNaviFunction->setIgnoreEvents(false);

		// destroy function
		this->closeFunction();

	}

}


