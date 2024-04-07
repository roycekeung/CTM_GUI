#include "DRemoveConnectorFunction.h"

// DISCO2_GUI lib
#include "PanelWidgets/DHintClosePanel.h"
#include "Functions/DBaseNaviFunction.h"
#include "GraphicsItems/DCellConnectorItem.h"
#include "GraphicsItems/DCellItem.h"
#include "GraphicsItems/DDemandSinkCellItem.h"
#include "GraphicsItems/DGraphicsScene.h"
#include "GraphicsItems/DGraphicsView.h"
#include "Utils/CoordTransform.h"
#include "DTabToolBar.h"
#include "DDockWidgetHandler.h"

// Qt lib
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QMessageBox>

// DISCO2_Core lib
#include "Jct_Link.h"
#include "VisInfo.h"
#include "Editor_Net.h"
#include "Net_Container.h"

const std::string DRemoveConnectorFunction::hintText{
"Left click on any connects to delete"
};


// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DRemoveConnectorFunction::DRemoveConnectorFunction() : QObject(nullptr) {}

DRemoveConnectorFunction::~DRemoveConnectorFunction() {}

void DRemoveConnectorFunction::initFunctionHandler() {
	// make a hint panel
	this->m_panel = new DHintClosePanel{ "Remove Connectors" };
	this->m_panel->setHintText(this->hintText.c_str());
	this->ref_dockWidgetHandler->setRightDockWidget(this->m_panel);
	QObject::connect(this->m_panel->getCloseButton(), &QPushButton::clicked, this, &DRemoveConnectorFunction::cancel);

	//turn off stuff
	this->ref_baseNaviFunction->setIgnoreEvents(true);
	this->ref_tabToolBar->setEnabled(false);

	// ensure none is selected at first
	this->ref_graphicsScene->clearSelection();
	this->ref_graphicsScene->removeAllDCellConnector();
	this->ref_graphicsScene->addAllConnectorOfVisibleItems();
}

// --- --- --- --- --- Checking and Remove Functions --- --- --- --- ---

void DRemoveConnectorFunction::checkAndRemoveConnector(DCellConnectorItem* item) {
	try {
		auto& cellConnectorEntry = this->ref_graphicsScene->getCellConnectorEntry_C2C(item);
		auto& cellJctConnectorEntry = this->ref_graphicsScene->getCellConnectorEntry_C2J(item);

		if (cellConnectorEntry.item) {
			this->ref_scn->get()->getNetEditor().cell_removeConnector(cellConnectorEntry.fromCellId, cellConnectorEntry.toCellId);
			this->ref_graphicsScene->removeDCellConnector_C2C(cellConnectorEntry.fromCellId, cellConnectorEntry.toCellId);
		}
		else if (cellJctConnectorEntry.item) {
			this->ref_scn->get()->getNetEditor().jct_disconnectCell(cellJctConnectorEntry.jctId, cellJctConnectorEntry.cellId);
			this->ref_graphicsScene->removeDCellConnector_C2J(cellJctConnectorEntry.cellId, cellJctConnectorEntry.jctId);
		}
		else
			this->m_panel->setFeedbackMessage("Cannot Identify Connector");
	}
	catch (std::exception& e) {
		this->m_panel->setFeedbackMessage(e.what());
	}
}

void DRemoveConnectorFunction::cancel() {

	this->ref_graphicsScene->reloadNet();
	this->ref_baseNaviFunction->setIgnoreEvents(false);
	this->ref_dockWidgetHandler->removeAndDeleteRightDockWidget();

	// make sure the button isn't toggled
	this->ref_tabToolBar->setEnabled(true);

	this->closeFunction();
}

// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---

bool DRemoveConnectorFunction::keyPressEvent(QKeyEvent* keyEvent) {
	if (keyEvent->key() == Qt::Key::Key_Escape) {
		keyEvent->accept();
		this->cancel();
		return true;
	}

	return false;
}

bool DRemoveConnectorFunction::mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	this->ref_graphicsScene->clearSelection();
	// switching the cursor icon
	DCellConnectorItem* item = this->connectorAt(mouseEvent->scenePos());
	if (item) {
		item->setSelected(true);
		this->ref_graphicsView->viewport()->setCursor(QCursor(QPixmap("./icons/trashbin_25px.png")));
	}
	else {
		this->ref_graphicsView->viewport()->setCursor(Qt::ArrowCursor);
	}

	mouseEvent->accept();
	return true;
}

bool DRemoveConnectorFunction::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	if (mouseEvent->button() == Qt::MouseButton::LeftButton) {
		this->checkAndRemoveConnector(this->connectorAt(mouseEvent->scenePos()));
	}

	//intercepts all scene event so notthing else gets selected
	mouseEvent->accept();
	return true;
}

// --- --- --- --- --- Private Util --- --- --- --- ---

DCellConnectorItem* DRemoveConnectorFunction::connectorAt(const QPointF& pt) {
	for (auto* topItem : this->ref_graphicsScene->items(pt)) {
		if (topItem->type() == (QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::CellConnectorItem))
			return dynamic_cast<DCellConnectorItem*>(topItem);
	}
	return nullptr;
}

