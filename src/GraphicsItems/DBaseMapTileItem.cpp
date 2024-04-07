#include "DBaseMapTileItem.h"

#include "DGraphicsScene.h"

#include <QPixmap>

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DBaseMapTileItem::DBaseMapTileItem(QPixmap& pixMap, TileInfo& tileInfo) : QGraphicsPixmapItem(pixMap), m_tileInfo(tileInfo) {
	//set default z-value (layer)
	this->setZValue(DGraphicsScene::ZValue_GraphicItemLayer::BaseMap + this->m_tileInfo.z);
	//Falgs
	this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable, false);
	this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, false);
}

DBaseMapTileItem::~DBaseMapTileItem() {}

// --- --- --- --- --- Getter --- --- --- --- ---

const TileInfo& DBaseMapTileItem::getTileInfo() const {
	return this->m_tileInfo;
}

// --- --- --- --- --- Override QGraphicsItem --- --- --- --- ---

int DBaseMapTileItem::type() const {
	return UserType + DGraphicsScene::ZValue_GraphicItemLayer::BaseMap;
}
