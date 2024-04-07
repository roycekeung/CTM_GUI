#include "DCellItem.h"

#include "DGraphicsScene.h"

#include <QPainter>
#include <QFontMetrics>

const QColor DCellItem::m_color = QColor{ 66, 135, 245 };
const QColor DCellItem::m_selectedColor = QColor{ 255, 132, 0 };
const QColor DCellItem::m_lineColor = Qt::GlobalColor::black;

const double DCellItem::cellWidthBuffer = 0.2;

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DCellItem::DCellItem(int cellId, int numOfLanes, double length) : QGraphicsRectItem(), m_cellId(cellId), m_numOfLanes(numOfLanes) {
	  
	//do the size of this thing
	this->updateSize(numOfLanes, length);
	//update to trigger redraw (I think)
	this->update();

	//set default z-value (layer)
	this->setZValue(DGraphicsScene::ZValue_GraphicItemLayer::CellItem);
	//Falgs
	this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable);
	this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, false);

	//style
	this->m_pen.setColor(DCellItem::m_lineColor);
	this->m_pen.setCosmetic(true);
	this->m_pen.setWidth(0);

}

DCellItem::~DCellItem() {

}

// --- --- --- --- --- Getters --- --- --- --- ---

int DCellItem::getCellId() {
	return this->m_cellId;
}

int DCellItem::getNumOfLanes() {
	return this->m_numOfLanes;
}

QPointF DCellItem::getConnectorExitPt() {
	qreal length = this->rect().width();
	return this->mapToScene({ (length / 2 - length / 3), 0 });
}

QPointF DCellItem::getConnectorEnterPt() {
	qreal length = this->rect().width();
	return this->mapToScene({ (length / 3 - length / 2), 0 });
}

// --- --- --- --- --- Editing Functions --- --- --- --- ---

void DCellItem::updateSize(int numOfLanes, double length) {
	//notify change first
	this->prepareGeometryChange();
	this->m_textCacheIsValid = false;
	this->m_numOfLanes = numOfLanes;
	//geometry change
	double halfWidth = ((numOfLanes * DGraphicsScene::laneWidth) / 2) - this->cellWidthBuffer;
	double halfLength = (length / 2) - this->cellWidthBuffer;
	this->setRect(-halfLength, -halfWidth, halfLength * 2, halfWidth * 2);
	this->update();
}

void DCellItem::setUseCustomColor(bool isUse, QColor color) {
	this->m_useCustomColor = isUse;
	this->m_customColor = std::move(color);
}

void DCellItem::setUseText(bool isUse, QString&& str) {
	this->m_useCustomText = isUse;
	this->m_customText = std::move(str);
}

// --- --- --- --- --- Override QGraphicsItem --- --- --- --- ---

int DCellItem::type() const {
	return UserType + DGraphicsScene::ZValue_GraphicItemLayer::CellItem;
}

void DCellItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	//style
	this->setPen(this->m_pen);
	this->m_brush.setColor(this->m_useCustomColor ? this->m_customColor : (this->isSelected() ? this->m_selectedColor : this->m_color));
	this->setBrush(this->m_brush);

	QGraphicsRectItem::paint(painter, option, widget);

	//custom text
	if (this->m_useCustomText) {
		//cache the pt size and pos adjustment
		if (!this->m_textCacheIsValid) {
			//figure out the pt size
			int flags = Qt::TextDontClip;
			QRect fontBoundRect = painter->fontMetrics().boundingRect(this->rect().toRect(), flags, "00.000");
			float xFactor = this->rect().width() * 0.95 / fontBoundRect.width();
			float yFactor = this->rect().height() * 0.95 / fontBoundRect.height();
			float factor = xFactor < yFactor ? xFactor : yFactor;
			QFont f = painter->font();
			this->m_ptSize = f.pointSizeF() * factor * 0.95;
			f.setPointSizeF(this->m_ptSize);
			painter->setFont(f);
			
			//get the final text rect and cal pos
			QRect finalFontBoundRect = painter->fontMetrics().boundingRect(this->rect().toRect(), flags, "00.000");
			this->m_textTransform.reset();
			double dx = (this->rect().width() - finalFontBoundRect.width()) / 2;
			double dy = -(this->rect().height() - finalFontBoundRect.height()) / 2;
			this->m_textTransform.translate(dx, dy);

			this->m_textCacheIsValid = true;
		}

		//font and pos
		QFont f = painter->font();
		f.setPointSizeF(this->m_ptSize);
		painter->setFont(f);
		painter->setTransform(this->m_textTransform, true);
		//color
		//source: https://mixable.blog/black-or-white-text-on-a-colour-background/
		int r, g, b;
		this->m_brush.color().getRgb(&r, &g, &b);
		painter->setPen((r * 0.299 + g * 0.587 + b * 0.114) >= 128 ? Qt::black : Qt::white);

		painter->drawText(this->rect().bottomLeft(), this->m_customText);
	}
}
