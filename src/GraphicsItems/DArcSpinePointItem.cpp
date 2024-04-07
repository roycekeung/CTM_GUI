#include "DArcSpinePointItem.h"

#include "DGraphicsScene.h"

const double DArcSpinePointItem::ptSize = 1;

const QColor DArcSpinePointItem::m_color = Qt::GlobalColor::yellow;
const QColor DArcSpinePointItem::m_selectedColor = Qt::GlobalColor::red;

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DArcSpinePointItem::DArcSpinePointItem() : QGraphicsPolygonItem() {
	//set default z-value (layer)
	this->setZValue(DGraphicsScene::ZValue_GraphicItemLayer::ArcSpinePt);
	//Falgs
	this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable);
	this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, true);

	//make thing
	this->m_poly << QPointF{ ptSize, 0 } << QPointF{ 0, ptSize } << QPointF{ -ptSize, 0 } << QPointF{ 0, -ptSize };
	this->setPolygon(this->m_poly);

	//do the color
	this->m_pen.setWidth(0);
	this->m_pen.setColor(this->m_color);
	this->setPen(this->m_pen);
}

DArcSpinePointItem::~DArcSpinePointItem() {

}

// --- --- --- --- --- Override QGraphicsItem --- --- --- --- ---

int DArcSpinePointItem::type() const {
	return UserType + DGraphicsScene::ZValue_GraphicItemLayer::ArcSpinePt;
}

void DArcSpinePointItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	//style
	this->m_brush.setColor(this->isSelected() ? this->m_selectedColor : this->m_color);
	this->setBrush(this->m_brush);
	//
	QGraphicsPolygonItem::paint(painter, option, widget);
}