#pragma once

#include <QGraphicsPolygonItem>
#include <QPen>
#include <QBrush>
#include <QPolygonF>

/**
 Preset 
*/
class DArcSpinePointItem : public QGraphicsPolygonItem {

private:

	const static double ptSize;

	const static QColor m_color;
	const static QColor m_selectedColor;

	QPen m_pen;
	QBrush m_brush{ Qt::BrushStyle::SolidPattern };

	QPolygonF m_poly;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DArcSpinePointItem();

	~DArcSpinePointItem();

	// --- --- --- --- --- Override QGraphicsItem --- --- --- --- ---

	int type() const override;

	virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0) override;

};
