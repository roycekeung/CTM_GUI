#pragma once

#include <Qobject>
#include <QGraphicsLineItem>

class DCellConnectorItem : public QObject, public QGraphicsLineItem {
	Q_OBJECT

private:

	QPointF m_startPt;
	QPointF m_endPt;

	QPolygonF m_arrowHeadPolygon;

	const static QColor m_color;
	const static QColor m_selectedColor;
	const static int m_arrowSize;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DCellConnectorItem(QPointF startPt, QPointF endPt);
	~DCellConnectorItem();

	void setStartPt(QPointF startPt);
	void setEndPt(QPointF endPt);

	// --- --- --- --- --- Override QGraphicsItem --- --- --- --- ---
	int type() const override;
	QRectF boundingRect() const override;
	QPainterPath shape() const override;

protected:
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

};

