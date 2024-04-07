#pragma once

#include <Qobject>
#include <QPen>
#include <QBrush>
#include <QGraphicsEllipseItem>

class DJctItem : public QObject, public QGraphicsEllipseItem {
	Q_OBJECT

private:

	int m_JctId;

	const static QColor m_color;
	const static QColor m_selectedColor;
	const static QColor m_lineColor;

	QPen m_pen;
	QBrush m_brush{ Qt::BrushStyle::SolidPattern };

public:
	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DJctItem(int jctId = -1);
	~DJctItem();

	// --- --- --- --- --- Getter --- --- --- --- ---
	
	int getJctId();

	QPointF getEdgeIntersectionPoint(QPointF sceneFromPt);

	// --- --- --- --- --- Override QGraphicsItem --- --- --- --- ---
	int type() const override;

	virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0) override;

};
