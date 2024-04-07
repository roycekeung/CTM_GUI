#include "DCellEditFunction.h"

#include <algorithm>

#include <QKeyEvent>

#include "PanelWidgets/Network/DCellPanel.h"
#include "DDockWidgetHandler.h"
#include "GraphicsItems/DGraphicsScene.h"
#include "GraphicsItems/DCellConnectorItem.h"
#include "GraphicsItems/DCellItem.h"
#include "Functions/DBaseNaviFunction.h"

//DISCO Core lib
#include <Cell_DemandSink.h>

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DCellEditFunction::DCellEditFunction(int cellId) : QObject(nullptr), m_cellId(cellId) {
}

DCellEditFunction::~DCellEditFunction() {
	//remove the cell panel
	this->ref_dockWidgetHandler->removeAndDeleteRightDockWidget();

	//re-enable baseNaviFunction
	this->ref_baseNaviFunction->setIgnoreEvents(false);
}

void DCellEditFunction::initFunctionHandler() {
	//setup the panel widget
	this->m_panelWidet = new DCellPanel(DCellPanel::Type_CellPanel::edit, this->ref_scn->get()->getNetwork().getCell(this->m_cellId),
		this->ref_graphicsScene->getToCellConnectors(this->m_cellId), this->ref_graphicsScene->getFromCellConnectors(this->m_cellId));
	this->ref_dockWidgetHandler->setRightDockWidget(this->m_panelWidet,	"Edit Cell");
	this->m_panelWidet->setEditable(true);
	this->m_panelWidet->setupEditButtons(this);

	//disable the baseNaviFunction
	this->ref_baseNaviFunction->setIgnoreEvents(true);
}

// --- --- --- --- --- Checking and creation Functions --- --- --- --- ---

void DCellEditFunction::checkAndEditCell() {
	//value checking for mrg div
	auto cellPtr = this->ref_scn->get()->getNetwork().getCell(this->m_cellId);
	if (cellPtr->getToCellIds().size() > 1 && this->ref_graphicsScene->getToCellConnectors(this->m_cellId).size() > 1) {
		double sum = 0;
		for (auto& rec : this->m_panelWidet->getDivTableInput())
			sum += rec.second;
		if (std::fabs(sum - 1) > std::numeric_limits<double>::epsilon()) {
			this->m_panelWidet->failMessage({ "Sum of Diverge Ratio must be 1" });
			//early exit as operation failed
			return;
		}
	}
	if (cellPtr->getFromCellIds().size() > 1 && this->ref_graphicsScene->getFromCellConnectors(this->m_cellId).size() > 1) {
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
		//update the cell
		this->ref_scn->get()->getNetEditor().cell_overrideParams(this->m_cellId, 
			this->m_panelWidet->getQInput(), this->m_panelWidet->getKInput(),
			this->m_panelWidet->getVfInput(), this->m_panelWidet->getWInput()
		);
		//update the mrg div ratios
		if (cellPtr->getToCellIds().size() > 1 && this->ref_graphicsScene->getToCellConnectors(this->m_cellId).size() > 1)
			for (auto& rec : this->m_panelWidet->getDivTableInput())
				this->ref_scn->get()->getNetEditor().cell_setDivergeRatio(this->m_cellId, rec.first, rec.second);
		if (cellPtr->getFromCellIds().size() > 1 && this->ref_graphicsScene->getFromCellConnectors(this->m_cellId).size() > 1)
			for (auto& rec : this->m_panelWidet->getMrgTableInput())
				this->ref_scn->get()->getNetEditor().cell_setMergeRatio(rec.first, this->m_cellId, rec.second);
		//VisInfo
		DISCO2_API::VisInfo tVisInfo = this->ref_scn->get()->getNetwork().getCell(this->m_cellId)->getVisInfo();
		tVisInfo.setName(this->m_panelWidet->getNameInput().toStdString());
		tVisInfo.setDetails(this->m_panelWidet->getDetailsInput().toStdString());
		this->ref_scn->get()->getNetEditor().cell_setVisInfo(this->m_cellId, std::move(tVisInfo));
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
	this->closeFunction();
}

void DCellEditFunction::cancel() {
	// end this function
	this->closeFunction();
}

// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---

bool DCellEditFunction::keyPressEvent(QKeyEvent* keyEvent) {
	if (keyEvent->key() == Qt::Key::Key_Escape) {
		keyEvent->accept();
		this->cancel();
		return true;
	}
	return false;
}
