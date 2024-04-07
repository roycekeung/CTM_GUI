#include "DDemandSinkCellCreateFunc.h"

//DISCO GUI stuff
#include "PanelWidgets/Network/DDemandSinkCellPanel.h"
#include "GraphicsItems/DDemandSinkCellItem.h"
#include "DDockWidgetHandler.h"
#include "GraphicsItems/DGraphicsScene.h"
#include "Functions/DBaseNaviFunction.h"
#include "Utils/DDefaultParamData.h"
#include "DTabToolBar.h"

//Qt lib
#include <QGraphicsSceneMouseEvent>
#include <QPushButton>
#include <QKeyEvent>
#include <QComboBox>

//DISCO Core lib
#include "Cell_DemandSink.h"
#include "VisInfo.h"


DDemandSinkCellCreateFunc::DDemandSinkCellCreateFunc(): QObject(nullptr) {

}

DDemandSinkCellCreateFunc::~DDemandSinkCellCreateFunc(){
	// re-able the baseNaviFunction
	this->ref_baseNaviFunction->setIgnoreEvents(false);
}


// --- --- --- --- --- override initFunctionHandler from I_Function --- --- --- --- ---
void DDemandSinkCellCreateFunc::initFunctionHandler() {
	//set up the graphics scene
	this->ref_graphicsScene->clearSelection();

	//setup the widget
	this->m_panelWidet = new DDemandSinkCellPanel{ DDemandSinkCellPanel::Type_Panel::create, this->ref_dParam };
	this->ref_dockWidgetHandler->setRightDockWidget(this->m_panelWidet, "Create Demand/ Sink Cell");

	// setup and connect to panel
	this->m_panelWidet->connectCreateFunction(this);
	this->m_panelWidet->getUI().FlowCap_doubleSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
	
	// disable the baseNaviFunction
	this->ref_baseNaviFunction->setIgnoreEvents(true);

	// disable the tabtoolbar
	this->ref_tabToolBar->setDisabled(true);
}


void DDemandSinkCellCreateFunc::checkAndCreate() {
	// ensure a clicked on scene before creation of demand sink cell
	if (this->m_clickStage != ClickStage::NoClick) {
		if (this->m_panelWidet->getFlowCapInput() == 0) {
			//print fail msg in the widget
			this->m_panelWidet->failMessage(QString("please fill in Flow Capacity "));
			return;
		}

		try {
			int tmp_CellID;

			// enum CellType { Demand =0 , Sink =1};
			// getFlowInCapacity == 0  means no flow in, does flow out; thus demand cell;  Type="DmdSnk" InQ="0"
			// getFlowInCapacity != 0  means does flow in, no flow out; thus Sink cell;   Type="DmdSnk" OutQ="0"
			if (this->m_panelWidet->getCellTypeInput() == 0) {
				// demand flow out
				tmp_CellID = this->ref_scn->get()->getDmdEditor().dmdCell_createDemandSink(0, this->m_panelWidet->getFlowCapInput());
				// used for setting color
				this->m_CellItem->setTypeOfCell(DDemandSinkCellItem::CellType::Demand);
			}
			else if (this->m_panelWidet->getCellTypeInput() == 1) {
				// flow into sink cell
				tmp_CellID = this->ref_scn->get()->getDmdEditor().dmdCell_createDemandSink(this->m_panelWidet->getFlowCapInput(), 0);
				// used for setting color
				this->m_CellItem->setTypeOfCell(DDemandSinkCellItem::CellType::Sink);
			}

			// save newly amended inputted params to default data struct bakcup
			this->ref_dParam->m_DmdCellFlowParams.SaturationFlow = this->m_panelWidet->getFlowCapInput();

			/// TODO DDemandSinkCellCreateFunc future coding ; input the Demand either continuous(uniform) or discrete(non-uniform) loading into core

			//saving visInfo, pos and rotation , name, detail
			DISCO2_API::VisInfo vis{};
			vis.setType(DISCO2_API::VisInfo::Type_VisInfo::Point);
			QPointF tPt1 = this->ref_coordTransform->transformToReal(this->m_CellItem->scenePos());
			DISCO2_API::VisInfo::VisInfo_Pt tVisInfoPt{ tPt1.x(), tPt1.y(), this->m_CellItem->rotation() };
			tVisInfoPt.m_Scale = this->m_CellItem->getScale(); 
			vis.addPoint(std::move(tVisInfoPt));
			vis.setName(this->m_panelWidet->getNameInput().toStdString());
			vis.setDetails(this->m_panelWidet->getDetailsInput().toStdString());
			this->ref_scn->get()->getNetEditor().cell_setVisInfo(tmp_CellID, std::move(vis));

			// add to record map in scene
			this->ref_graphicsScene->addDmdSnkItem(tmp_CellID);

			//close if succeeded
			this->cancel();
		}
		catch (std::exception& e) {
			//print fail msg in the widget
			this->m_panelWidet->failMessage({ e.what() });
		}

	}
	else {
		//print fail msg in the widget
		this->m_panelWidet->failMessage(QString("please clicked on scene to create cell"));
	}
}

void DDemandSinkCellCreateFunc::cancel() {
	//delete all the graphical things
	if (this->m_clickStage != ClickStage::NoClick) {
		//delete item in scene and pointer in func class
		this->ref_graphicsScene->removeItem(this->m_CellItem);
		delete this->m_CellItem;
	}

	//remove the demand sink cell panel
	this->ref_dockWidgetHandler->removeAndDeleteRightDockWidget();

	// clear selection after finish creating or not yet created
	this->ref_graphicsScene->clearSelection();

	// enable the tabtoolbar
	this->ref_tabToolBar->setDisabled(false);

	// end this function
	this->closeFunction();
}

// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---

bool DDemandSinkCellCreateFunc::keyPressEvent(QKeyEvent* keyEvent) {
	if (keyEvent->key() == Qt::Key::Key_Escape) {
		this->cancel();
		return true;
	}
	else if (keyEvent->key()== Qt::Key::Key_Enter){
		checkAndCreate();
		return true;
	}
	// continue to handle other events
	return false;
}

bool DDemandSinkCellCreateFunc::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	if (mouseEvent->button() == Qt::MouseButton::LeftButton)
		switch (this->m_clickStage) {
		case created:
			// do nth
			break;
		default:
			this->ref_graphicsScene->clearSelection();
			break;

		case NoClick:{		//waitng for first click
			// firstly create graphicitem 
			this->m_CellItem = new DDemandSinkCellItem();
			this->m_CellItem->setMovableRotatable(true);
			this->m_CellItem->setPos(mouseEvent->scenePos());
			this->m_CellItem->setSelected(true);
			this->m_CellItem->update();

			this->m_panelWidet->connectItemToConfirmButton(this->m_CellItem);

			// add tmp graphicitem into scene
			this->ref_graphicsScene->addItem(this->m_CellItem);
			// Cell ID is not ready yet, so update and add to record map in scene later
			this->ref_graphicsScene->update();

			//change state
			this->m_clickStage = created;

			//still want to work on event dont pass to parent widget
			mouseEvent->accept();
			 
			// continue to handle other events
			return false;
			break;
		}
		}//end of m_clickStage switch
	// continue to handle other events
	return false;
}


