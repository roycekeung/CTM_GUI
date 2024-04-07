#include "DJctItem.h"

#include "DGraphicsScene.h"

#include <QPainter>

const QColor DJctItem::m_color = QColor{ 66, 135, 245 };
const QColor DJctItem::m_selectedColor = QColor{ 255, 132, 0 };
const QColor DJctItem::m_lineColor = Qt::GlobalColor::black;

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DJctItem::DJctItem(int jctId) : QObject(nullptr), 
    QGraphicsEllipseItem(-DGraphicsScene::laneWidth, -DGraphicsScene::laneWidth, DGraphicsScene::laneWidth*2, DGraphicsScene::laneWidth*2),
    m_JctId(jctId) {

    //set default z-value (layer)
    this->setZValue(DGraphicsScene::ZValue_GraphicItemLayer::JctItem);

    this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable);
    this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, false);

    //style
    this->m_pen.setColor(DJctItem::m_lineColor);
    this->m_pen.setCosmetic(true);
    this->m_pen.setWidth(0);
}

DJctItem::~DJctItem() {

}

// --- --- --- --- --- Getter --- --- --- --- ---

int DJctItem::getJctId() {
    return this->m_JctId;
}

QPointF DJctItem::getEdgeIntersectionPoint(QPointF sceneFromPt) {
    //TODO figure out how to cal the edge intersection point
    return this->pos();
}

// --- --- --- --- --- Override QGraphicsItem --- --- --- --- ---

int DJctItem::type() const {
    return UserType + DGraphicsScene::ZValue_GraphicItemLayer::JctItem;
}

void DJctItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    //style
    this->setPen(this->m_pen);
    this->m_brush.setColor(this->isSelected() ? this->m_selectedColor : this->m_color);
    this->setBrush(this->m_brush);

    QGraphicsEllipseItem::paint(painter, option, widget);
}