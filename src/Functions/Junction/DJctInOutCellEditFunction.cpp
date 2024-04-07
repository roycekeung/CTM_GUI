#include "DJctInOutCellEditFunction.h"

#include <algorithm>

#include "PanelWidgets/Junction/DJctInOutCellPanel.h"
#include "DDockWidgetHandler.h"
#include "Functions/DBaseNaviFunction.h"
#include "Functions/Junction/DJunctionModeFunction.h"
#include "GraphicsItems/DGraphicsScene.h"
#include "GraphicsItems/DCellConnectorItem.h"
#include "GraphicsItems/DPseudoJctLinkItem.h"
#include "GraphicsItems/DCellItem.h"

#include <QKeyEvent>

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DJctInOutCellEditFunction::DJctInOutCellEditFunction(int jctId, int cellId) : QObject(nullptr), m_jctId(jctId), m_cellId(cellId) {}

DJctInOutCellEditFunction::~DJctInOutCellEditFunction() {}

void DJctInOutCellEditFunction::initFunctionHandler() {
	//setup the panel widget
	this->m_panelWidet = new DJctInOutCellPanel(DJctInOutCellPanel::Type_CellPanel::edit, 
		this->ref_scn->get()->getNetwork().getCell(this->m_cellId), this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId),
		std::unordered_map<int, DPseudoJctLinkItem*>{ this->ref_graphicsScene->getPseudoJctLinksInJct(this->m_jctId) });
	this->ref_dockWidgetHandler->setRightDockWidget(this->m_panelWidet, "Edit Cell");
	this->m_panelWidet->setEditable(true);
	this->m_panelWidet->setupEditButtons(this);

	//disable the JctModeFunction
	this->ref_baseNaviFunction->getJctModeFunction()->setIgnoreEvents(true);
}

// --- --- --- --- --- Checking and creation Functions --- --- --- --- ---

void DJctInOutCellEditFunction::checkAndEditCell() {
	//value checking for mrg div
	auto cellPtr = this->ref_scn->get()->getNetwork().getCell(this->m_cellId);
	auto& jct = this->ref_scn->get()->getNetwork().getJctNode(this->m_jctId);
	if (jct.getInCells().count(this->m_cellId) && cellPtr->getToCellIds().size() > 1) {
		double sum = 0;
		for (auto& rec : this->m_panelWidet->getDivTableInput())
			sum += rec.second;
		if (std::fabs(sum - 1) > std::numeric_limits<double>::epsilon()) {
			this->m_panelWidet->failMessage({ "Sum of Diverge Ratio must be 1" });
			//early exit as operation failed
			return;
		}
	}
	else if (jct.getOutCells().count(this->m_cellId) && cellPtr->getFromCellIds().size() > 1) {
		double sum = 0;
		for (auto& rec : this->m_panelWidet->getMrgTableInput())
			sum += rec.second;
		if (std::fabs(sum - 1) > std::numeric_limits<double>::epsilon()) {
			this->m_panelWidet->failMessage({ "Sum of Merge Ratio must be 1" });
			//early exit as operation failed
			return;
		}
	}

	try {
		//update the mrg div ratios
		if (jct.getInCells().count(this->m_cellId) && cellPtr->getToCellIds().size() > 1)
			for (auto& rec : this->m_panelWidet->getDivTableInput())
				this->ref_scn->get()->getNetEditor().jct_setToJctLinkRatio(this->m_jctId, this->m_cellId, rec.first, rec.second);
		else if (jct.getOutCells().count(this->m_cellId) && cellPtr->getFromCellIds().size() > 1)
			for (auto& rec : this->m_panelWidet->getMrgTableInput())
				this->ref_scn->get()->getNetEditor().jct_setFromJctLinkRatio(this->m_jctId, this->m_cellId, rec.first, rec.second);
	}
	catch (std::exception& e) {
		//print fail msg in the widget
		this->m_panelWidet->failMessage({ e.what() });
		//early exit as operation failed
		return;
	}

	//update graphics
	this->ref_graphicsScene->updateDCellItem(this->m_cellId);
	// end this function
	this->cancel();
}

void DJctInOutCellEditFunction::cancel() {
	//remove the cell panel
	this->ref_dockWidgetHandler->removeAndDeleteRightDockWidget();

	//re-enable JctModeFunction
	this->ref_baseNaviFunction->getJctModeFunction()->setIgnoreEvents(false);

	// end this function
	this->closeFunction();
}

// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---

bool DJctInOutCellEditFunction::keyPressEvent(QKeyEvent* keyEvent) {
	if (keyEvent->key() == Qt::Key::Key_Escape) {
		this->cancel();
		return true;
	}
	return false;
}
