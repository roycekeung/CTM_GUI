#include "DCellConnectorItem.h"

#include <QPen>
#include <QtMath>
#include <QPainter>

#include "DGraphicsScene.h"

// --- --- --- --- --- Init Const Static Stuff --- --- --- --- ---

const QColor DCellConnectorItem::m_color = Qt::GlobalColor::black;
const QColor DCellConnectorItem::m_selectedColor = Qt::GlobalColor::red;
const int DCellConnectorItem::m_arrowSize = 1;

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DCellConnectorItem::DCellConnectorItem(QPointF startPt, QPointF endPt) : m_startPt(startPt), m_endPt(endPt) {
    this->setLine({ this->m_startPt, this->m_endPt });
    //set default z-value (layer)
    this->setZValue(DGraphicsScene::ZValue_GraphicItemLayer::CellConnectorItem);
    //Flags
    this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable);
    this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, false);
    //Style
    this->setPen(QPen(this->m_color, 0.5, Qt::SolidLine, Qt::FlatCap, Qt::PenJoinStyle::MiterJoin));
}

DCellConnectorItem::~DCellConnectorItem() {}

void DCellConnectorItem::setStartPt(QPointF startPt) {
    this->prepareGeometryChange();
    this->m_startPt = startPt;
    this->setLine({ this->m_startPt, this->m_endPt });
}

void DCellConnectorItem::setEndPt(QPointF endPt) {
    this->prepareGeometryChange();
    this->m_endPt = endPt;
    this->setLine({ this->m_startPt, this->m_endPt });
    this->update();
}

// --- --- --- --- --- Override QGraphicsItem --- --- --- --- ---

int DCellConnectorItem::type() const {
	return UserType + DGraphicsScene::ZValue_GraphicItemLayer::CellConnectorItem;
}

QRectF DCellConnectorItem::boundingRect() const {
	qreal extra = (pen().width() + this->m_arrowSize) / 2.0;

	return QRectF(line().p1(), QSizeF(line().p2().x() - line().p1().x(),
		line().p2().y() - line().p1().y()))
		.normalized()
		.adjusted(-extra, -extra, extra, extra);
}

QPainterPath DCellConnectorItem::shape() const {
	QPainterPath path = QGraphicsLineItem::shape();
	path.addPolygon(m_arrowHeadPolygon);
	return path;
}

void DCellConnectorItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    QPen myPen = pen();
    myPen.setColor(this->isSelected() ? this->m_selectedColor : this->m_color);
    painter->setPen(myPen);
    painter->setBrush(this->isSelected() ? this->m_selectedColor : this->m_color);

    setLine(QLineF(this->m_startPt, this->m_endPt));
    double angle = std::atan2(-line().dy(), line().dx());

    QPointF arrowP1 = this->line().p2() - QPointF(sin(angle + M_PI / 3) * this->m_arrowSize, cos(angle + M_PI / 3) * this->m_arrowSize);
    QPointF arrowP2 = this->line().p2() - QPointF(sin(angle + M_PI - M_PI / 3) * this->m_arrowSize, cos(angle + M_PI - M_PI / 3) * this->m_arrowSize);

    this->m_arrowHeadPolygon.clear();
    this->m_arrowHeadPolygon << this->line().p2() << arrowP1 << arrowP2;

    painter->drawLine(this->line());
    painter->drawPolygon(this->m_arrowHeadPolygon);
}