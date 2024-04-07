#include "DDeleteJctLinkFunction.h"

#include <QPushButton>
#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItem>

#include "Functions/DBaseNaviFunction.h"
#include "Functions/Junction/DJunctionModeFunction.h"
#include "GraphicsItems/DGraphicsScene.h"
#include "GraphicsItems/DGraphicsView.h"
#include "GraphicsItems/DPseudoJctLinkItem.h"
#include "DDockWidgetHandler.h"
#include "DTabToolBar.h"
#include "PanelWidgets/DHintClosePanel.h"

const std::string DDeleteJctLinkFunction::hintText{
"Hint text for delete Jct Link\r\n\
1 \r\n\
1 \r\n"
};

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DDeleteJctLinkFunction::DDeleteJctLinkFunction(QPushButton* deleteJctLinkButton) : QObject(nullptr), I_Function(), m_deleteButton(deleteJctLinkButton) {}

DDeleteJctLinkFunction::~DDeleteJctLinkFunction() {}

void DDeleteJctLinkFunction::initFunctionHandler() {
	//deal with background stuff
	this->ref_tabToolBar->setEnabled(false);
	this->ref_baseNaviFunction->setIgnoreEvents(true);
	this->ref_baseNaviFunction->getJctModeFunction()->setIgnoreEvents(true);

	//grab the current jct
	this->m_jctId = this->ref_baseNaviFunction->getJctModeFunction()->getCurrentJctId();
	this->ref_graphicsScene->setShowDetailJctLinkView(this->m_jctId, false);

	//do the panel widget
	this->m_panel = new DHintClosePanel{"Delete Junction Link"};
	this->ref_dockWidgetHandler->setRightDockWidget(this->m_panel);
	QObject::connect(this->m_panel->getCloseButton(), &QPushButton::clicked, this, &DDeleteJctLinkFunction::cancel);
}

void DDeleteJctLinkFunction::cancel() {
	//deal with background stuff
	this->ref_tabToolBar->setEnabled(true);
	this->ref_baseNaviFunction->setIgnoreEvents(false);
	this->ref_baseNaviFunction->getJctModeFunction()->setIgnoreEvents(false);
	//the panel widget
	this->ref_dockWidgetHandler->removeAndDeleteRightDockWidget();
	
	this->closeFunction();
}

// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---

bool DDeleteJctLinkFunction::keyPressEvent(QKeyEvent* keyEvent) {
	if (keyEvent->key() == Qt::Key::Key_Escape) {
		keyEvent->accept();
		this->cancel();
		return true;
	}

	return false;
}

bool DDeleteJctLinkFunction::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	QGraphicsItem* item = this->ref_graphicsScene->itemAt(mouseEvent->scenePos(), {});
	this->ref_graphicsScene->clearSelection();

	if (item && item->type() == QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::PseudoJLinkItem) {
		for (auto& pJctLinkEntry : this->ref_graphicsScene->getPseudoJctLinksInJct(this->m_jctId)) {
			if (pJctLinkEntry.second == dynamic_cast<DPseudoJctLinkItem*>(item)) {
				try {
					//delete the thing
					this->ref_scn->get()->getNetEditor().jctLink_delete(this->m_jctId, pJctLinkEntry.first);

					//have graphics scene make it
					this->ref_graphicsScene->clearSelection();
					this->ref_graphicsScene->removeAllDPseudoJLinkItemInJct(this->m_jctId);
					this->ref_graphicsScene->removeAllJLinkItemInJct(this->m_jctId);
					this->ref_graphicsScene->addAllDPseudoJLinkItemInJct(this->m_jctId);
					this->ref_graphicsScene->addAllJLinkItemInJct(this->m_jctId);
					this->ref_graphicsScene->setShowDetailJctLinkView(this->m_jctId, false);
					this->ref_graphicsScene->update();
				}
				catch (std::exception& e) {
					//print fail msg in the widget
					this->m_panel->setFeedbackMessage({ e.what() });
				}

				break;
			}
		}
			
		this->ref_graphicsView->viewport()->setCursor(Qt::ArrowCursor);
		mouseEvent->accept();
		return true;
	}
	return false;
}

bool DDeleteJctLinkFunction::mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	// switching the cursor icon
	QGraphicsItem* item = this->ref_graphicsScene->itemAt(mouseEvent->scenePos(), {});
	this->ref_graphicsScene->clearSelection();

	if (item) {
		if (item->type() == QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::PseudoJLinkItem) {
			item->setSelected(true);
			this->ref_graphicsView->viewport()->setCursor(QCursor(QPixmap("./icons/trashbin_25px.png")));

			//DESIGN why not CrossCursor??? JLo
			//Qt::CursorShape::CrossCursor
		}
		else {
			this->ref_graphicsView->viewport()->setCursor(Qt::ArrowCursor);
		}

		mouseEvent->accept();
		return true;
	}
	return false;
}
