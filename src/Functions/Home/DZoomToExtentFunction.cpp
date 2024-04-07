#include "DZoomToExtentFunction.h"

#include <QGraphicsItem>

// DISCO2_GUI lib
#include "GraphicsItems/DGraphicsScene.h"
#include "GraphicsItems/DGraphicsView.h"
#include "Functions/DBaseMapFunction.h"

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---
DZoomToExtentFunction::DZoomToExtentFunction() {}

DZoomToExtentFunction::~DZoomToExtentFunction() {}

// --- --- --- --- --- override initFunctionHandler from I_Function --- --- --- --- ---
void DZoomToExtentFunction::initFunctionHandler() {
	//get all the items
	QList<QGraphicsItem*> items;
	if (this->ref_graphicsScene->selectedItems().isEmpty())
		items = this->ref_graphicsScene->items();
	else
		items = this->ref_graphicsScene->selectedItems();
	//check there are stuff in the graphicsScene
	if (!items.isEmpty()) {
		//get a reference point
		auto item1 = items.front();
		double minX = item1->mapToScene(item1->boundingRect().center()).x(), minY = item1->mapToScene(item1->boundingRect().center()).y();
		double maxX = item1->mapToScene(item1->boundingRect().center()).x(), maxY = item1->mapToScene(item1->boundingRect().center()).y();

		//figure out the bounding rect without the basemap
		bool onlyBaseMap = true;
		for (auto item : items) {
			if (item->type() != QGraphicsItem::UserType + DGraphicsScene::ZValue_GraphicItemLayer::BaseMap) {
				onlyBaseMap = false;
				for (auto& pt : item->mapToScene(item->boundingRect())) {
					if (pt.x() < minX)
						minX = pt.x();
					else if (pt.x() > maxX)
						maxX = pt.x();
					if (pt.y() < minY)
						minY = pt.y();
					else if (pt.y() > maxY)
						maxY = pt.y();
				}
			}
		}
		//set the graphics view to new scene rect
		QRectF sceneRect;
		if (onlyBaseMap)
			sceneRect = this->ref_graphicsScene->sceneRect();
		else
			sceneRect = { QPointF{minX, minY}, QPointF{maxX, maxY} };
		this->ref_graphicsView->fitInView(sceneRect, Qt::AspectRatioMode::KeepAspectRatio);
		this->ref_graphicsView->viewport()->update();

		this->ref_baseMapFunction->sceneMoved();
	}

	this->closeFunction();
}
