#pragma once

#include <QGraphicsPixmapItem>

#include "Utils/I_TileGenerator.h"

class QPixmap;

class DBaseMapTileItem : public QGraphicsPixmapItem {
private:

	const TileInfo m_tileInfo;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DBaseMapTileItem(QPixmap& pixMap, TileInfo& tileInfo);

	~DBaseMapTileItem();

	// --- --- --- --- --- Getter --- --- --- --- ---

	const TileInfo& getTileInfo() const;

	// --- --- --- --- --- Override QGraphicsItem --- --- --- --- ---

	int type() const override;

};
