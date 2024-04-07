#include "DDemandSinkCellViewFunc.h"

//DISCO GUI stuff
#include "PanelWidgets/Network/DDemandSinkCellPanel.h"
#include "GraphicsItems/DDemandSinkCellItem.h"
#include "DDockWidgetHandler.h"
#include "GraphicsItems/DGraphicsScene.h"
#include "Utils/MouseWheelWidgetAdjustmentGuard.h"
#include "Functions/DBaseNaviFunction.h"
#include "Functions/Network/DDemandSinkCellEditFunc.h"
#include "DFunctionHandler.h"

//Qt lib
#include <QGraphicsSceneMouseEvent>
#include <QPushButton>
#include <QKeyEvent>
#include <QComboBox>

//DISCO Core lib
#include "Cell_DemandSink.h"
#include "VisInfo.h"


void DDemandSinkCellViewFunc::showDemandInputTable() {
	if (this->ref_scn->get()->getDemand().getDemandSetIds().empty()) {
		return;
	}
	// link up the dmdset with the input table
	for (int DemSetID : this->ref_scn->get()->getDemand().getDemandSetIds()) {
		this->m_panelWidet->getUI().Plan_comboBox->addItem(QString("%1").arg(DemSetID));
	}
}

// --- --- --- --- --- internal Functions --- --- --- --- ---
DDemandSinkCellViewFunc::DDemandSinkCellViewFunc(int cellId) : tmp_CellID(cellId), QObject(nullptr) {

}

DDemandSinkCellViewFunc::~DDemandSinkCellViewFunc(){	

}


// --- --- --- --- --- override initFunctionHandler from I_Function --- --- --- --- ---
void DDemandSinkCellViewFunc::initFunctionHandler() {
	const DISCO2_API::Cell_DemandSink* m_DmdSnkCellFromCore = this->ref_scn->get()->getNetwork().getCell_DemandSink(this->tmp_CellID);

	// ensure intact selection of the Demand Sink Cell graphicitem then maybe plot with connecter 
	this->ref_graphicsScene->removeAllDCellConnector();
	this->ref_graphicsScene->clearSelection();
	this->ref_graphicsScene->getDmdSnkCellItem(this->tmp_CellID)->setSelected(true);
	this->ref_graphicsScene->addConnectorsOfSelectedItems();

	//grab the item that's clicked on
	DDemandSinkCellItem* topItem = dynamic_cast<DDemandSinkCellItem*>(this->ref_graphicsScene->selectedItems().first());

	//setup the widget; by double confirmation; getFlowInCapacity check got some flaws; but force flow cap input has to be non-0 then solved 
	if (m_DmdSnkCellFromCore->getFlowInCapacity() == 0 && topItem->getTypeOfCell() == DDemandSinkCellItem::CellType::Demand) {
		this->m_panelWidet = new DDemandSinkCellPanel{ DDemandSinkCellPanel::Type_Panel::view, this->ref_dParam, DDemandSinkCellPanel::CellType::Demand, m_DmdSnkCellFromCore,
		this->ref_graphicsScene->getToCellConnectors(this->tmp_CellID), this->ref_graphicsScene->getFromCellConnectors(this->tmp_CellID) };
		this->ref_dockWidgetHandler->setRightDockWidget(this->m_panelWidet, "View Demand Cell", true);
	}
	else if (m_DmdSnkCellFromCore->getFlowInCapacity() != 0 && topItem->getTypeOfCell() == DDemandSinkCellItem::CellType::Sink) {
		this->m_panelWidet = new DDemandSinkCellPanel{ DDemandSinkCellPanel::Type_Panel::view, this->ref_dParam, DDemandSinkCellPanel::CellType::Sink, m_DmdSnkCellFromCore,
		this->ref_graphicsScene->getToCellConnectors(this->tmp_CellID), this->ref_graphicsScene->getFromCellConnectors(this->tmp_CellID) };
		this->ref_dockWidgetHandler->setRightDockWidget(this->m_panelWidet, "View Sink Cell", true);
	}

	// setup and connect to panel
	this->m_panelWidet->connectViewFunction(this);

	// setup and DemandInputTable widget
	showDemandInputTable();

	// close this function when dockwidget close
	QObject::connect(this->ref_dockWidgetHandler, &DDockWidgetHandler::rightDockWidgetClosed, this, &DDemandSinkCellViewFunc::cancel);

}

// --- --- --- --- --- update and show Functions --- --- --- --- ---
void DDemandSinkCellViewFunc::updateDemandInputTablefromCore(const QString& dmd_setID) {
	// enum CellType { Demand =0 , Sink =1};
	if (this->m_panelWidet->getCellTypeInput() == 1) {
		// Sink Cell; dont have DemandInputTable
		return;
	}

	if (this->ref_scn->get()->getDemand().getDemandSet(dmd_setID.toInt()).hasDmdLoader(this->tmp_CellID) == false) {
		// dmdset dont have the demand loader yet
		return;
	}

	// get DemandLoaders directly from core
	auto DemandLoaders_inf = this->ref_scn->get()->getDemand().getDemandSet(dmd_setID.toInt()).getAllIntervals_continuousDmdLoad(this->tmp_CellID);
		
	/// remove all item data then re-build
	this->m_panelWidet->getUI().DemandInput_tableWidget->setRowCount(0);

	// define the table row number
	this->m_panelWidet->getUI().DemandInput_tableWidget->setRowCount(DemandLoaders_inf.size());

	// update the DemandInputTable
	int current_row = 0;
	for (auto DemandLoader : DemandLoaders_inf) {
		int start_time = std::get<0>(DemandLoader);
		int end_time = std::get<1>(DemandLoader);
		double rate = std::get<2>(DemandLoader);

		// defualt table initial set up
		QSpinBox* startTimeCell = new QSpinBox(this->m_panelWidet->getUI().DemandInput_tableWidget);
		startTimeCell->setMaximum(INT_MAX);
		startTimeCell->setValue(start_time);
		startTimeCell->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
		startTimeCell->setButtonSymbols(QAbstractSpinBox::NoButtons);
		this->m_panelWidet->getUI().DemandInput_tableWidget->setCellWidget(current_row, 0, startTimeCell);
		startTimeCell->setToolTip(tr("load from core"));
		startTimeCell->setReadOnly(true);  // not editable but can select
		startTimeCell->setFrame(false);  // no frame

		QSpinBox* endTimeCell = new QSpinBox(this->m_panelWidet->getUI().DemandInput_tableWidget);
		endTimeCell->setMaximum(INT_MAX);
		endTimeCell->setValue(end_time);
		endTimeCell->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
		endTimeCell->setButtonSymbols(QAbstractSpinBox::NoButtons);
		this->m_panelWidet->getUI().DemandInput_tableWidget->setCellWidget(current_row, 1, endTimeCell);
		endTimeCell->setToolTip(tr("load from core"));
		endTimeCell->setReadOnly(true);  // not editable but can select
		endTimeCell->setFrame(false);  // no frame

		QDoubleSpinBox* rateCell = new QDoubleSpinBox(this->m_panelWidet->getUI().DemandInput_tableWidget);
		rateCell->setMaximum(DBL_MAX);
		rateCell->setValue(rate*3600);
		rateCell->setDecimals(2);
		rateCell->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
		rateCell->setButtonSymbols(QAbstractSpinBox::NoButtons);
		this->m_panelWidet->getUI().DemandInput_tableWidget->setCellWidget(current_row, 2, rateCell);
		rateCell->setToolTip(tr("load from core"));
		rateCell->setReadOnly(true);  // not editable but can select
		rateCell->setFrame(false);  // no frame

		current_row++;
	}
	// update the current selected combobox dmdsetID
	this->previous_dmdsetID = dmd_setID.toInt();


}

void DDemandSinkCellViewFunc::cancel() {
	// end this function
	this->closeFunction();
}

// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---

bool DDemandSinkCellViewFunc::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	if (this->ref_baseNaviFunction->getIsNetEditMode() && this->ref_graphicsScene->items(mouseEvent->scenePos()).size()) {
		auto* topItem = this->ref_graphicsScene->items(mouseEvent->scenePos()).first();
		if (topItem->type() == QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::DmdSnkCellItem) {
			this->ref_functionHandler->addFunction(new DDemandSinkCellEditFunc{ this->tmp_CellID });
			this->closeFunction();
			return true;
		}
	}
	return false;
}


