#include "DPolyArcItem.h"

#include "DGraphicsScene.h"
#include "DCellItem.h"

#include <QPainter>
#include <QPainterPathStroker>
#include <qmath.h>

const QColor DPolyArcItem::m_color = QColor{ 97, 97, 97 };
const QColor DPolyArcItem::m_arrowColor = QColor{ 251, 255, 0 };
const QColor DPolyArcItem::m_selectedColor = Qt::GlobalColor::red;

const double DPolyArcItem::arcBuffer = 0.5;
const double DPolyArcItem::arrowLength = 2.0;
const double DPolyArcItem::arrowWidth = 0.7;

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DPolyArcItem::DPolyArcItem(int arcId, double cellLength, int numOfLanes, int numOfRows, const QList<QPointF>& ptList) :
		QGraphicsPathItem(nullptr), m_arcId(arcId), m_cellLength(cellLength), m_numOfLanes(numOfLanes), m_numOfRows(numOfRows) {
	//make the path
	this->updatePos(ptList);

	//set default z-value (layer)
	this->setZValue(DGraphicsScene::ZValue_GraphicItemLayer::ArcItem);
	//Flags
	this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable);
	this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, false);
}

DPolyArcItem::~DPolyArcItem() {}

// --- --- --- --- --- Editing Functions --- --- --- --- ---

void DPolyArcItem::updateSize(double cellLength, int numOfLanes, int numOfRows) {
	this->m_cellLength = cellLength;
	this->m_numOfLanes = numOfLanes;
	this->m_numOfRows = numOfRows;

	this->calDrawingBits();

	//assume always need to update everything
	this->prepareGeometryChange();
	this->update();
}

void DPolyArcItem::updatePos(QList<QPointF> ptList) {
	//make path
	this->m_thinPath.clear();
	if (ptList.size()) {
		this->m_thinPath = QPainterPath{ ptList.front() };
		ptList.pop_front();
		for (auto pt : ptList)
			this->m_thinPath.lineTo(pt);
	}

	this->calDrawingBits();

	//assume always need to update everything
	this->prepareGeometryChange();
	this->update();
}

void DPolyArcItem::setCellItemRotPos(const QList<QList<DCellItem*>>& cellItems) const {
	//assign the centroid locations
	for (int row = 0; row < this->m_numOfRows && row < cellItems.size(); row++)
		this->setCellItemRotPos(row, cellItems.at(row));
}

void DPolyArcItem::setCellItemRotPos(int row, const QList<DCellItem*>& cellItems) const {
	//center point
	qreal atPercent = this->m_thinPath.percentAtLength(this->m_thinPath.length() - (row + 0.5) * this->m_cellLength);
	QPointF centerPt = this->m_thinPath.pointAtPercent(atPercent);
	qreal angleD = this->m_thinPath.angleAtPercent(atPercent);
	qreal angleR = qDegreesToRadians(angleD);

	//keep track where the lane needs to be at
	int numOfCells = cellItems.size();
	int laneDone = 0;
	for (auto cellItem : cellItems) {
		double thisW = -(0.5 * (cellItem->getNumOfLanes() - this->m_numOfLanes) + laneDone) * DGraphicsScene::laneWidth;
		cellItem->setPos(centerPt + QPointF(-thisW * std::sin(angleR), -thisW * std::cos(angleR)));
		cellItem->setRotation(-angleD);
		laneDone += cellItem->getNumOfLanes();
		cellItem->update();
	}
}

// --- --- --- --- --- Getter --- --- --- --- ---

int DPolyArcItem::getArcId() const {
	return this->m_arcId;
}

int DPolyArcItem::getRowClosestToPt(QPointF scenePt) const {
	int out = 0;
	double minLength = DBL_MAX;
	for (int row = 0; row < this->m_numOfRows; row++) {
		//center point
		qreal atPercent = this->m_thinPath.percentAtLength(this->m_thinPath.length() - (row + 0.5) * this->m_cellLength);
		QPointF centerPt = this->m_thinPath.pointAtPercent(atPercent);
		if ((centerPt - scenePt).manhattanLength() < minLength) {
			out = row;
			minLength = (centerPt - scenePt).manhattanLength();
		}
	}
	return out;
}

double DPolyArcItem::getRowRot(int row) const {
	qreal atPercent = this->m_thinPath.percentAtLength(this->m_thinPath.length() - row * this->m_cellLength);
	return -this->m_thinPath.angleAtPercent(atPercent);
}

QPointF DPolyArcItem::getRowCenterPt(int row) const {
	qreal atPercent = this->m_thinPath.percentAtLength(this->m_thinPath.length() - (row + 0.5) * this->m_cellLength);
	return this->m_thinPath.pointAtPercent(atPercent);
}

// --- --- --- --- --- Override QGraphicsItem --- --- --- --- ---

int DPolyArcItem::type() const {
	return UserType + DGraphicsScene::ZValue_GraphicItemLayer::ArcItem;
}

void DPolyArcItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	//set width and use default paint for path
	QPen pen{ (this->isSelected() ? this->m_selectedColor : this->m_color),
		0, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin };
	painter->setPen(pen);
	painter->setBrush((this->isSelected() ? this->m_selectedColor : this->m_color));
	painter->drawPath(this->path());

	// draw half arrows
	painter->setPen(this->m_arrowColor);
	painter->setPen(QPen(this->m_arrowColor, 0, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
	painter->setBrush(this->m_arrowColor);
	for (auto pointyBit : this->m_pointyBits)
		painter->drawPath(pointyBit);
}

// --- --- --- --- --- Private Util --- --- --- --- ---

void DPolyArcItem::calDrawingBits() {
	double width = this->m_numOfLanes * DGraphicsScene::laneWidth + arcBuffer * 2;

	//the underlying arc
	QPainterPathStroker stroker{};
	stroker.setCapStyle(Qt::FlatCap);
	stroker.setWidth(width);
	this->setPath(stroker.createStroke(this->m_thinPath));

	//the pointy bits
	this->m_pointyBits.clear();
	for (int row = 1; row < this->m_numOfRows; row++) {
		//center point
		qreal atPercent = this->m_thinPath.percentAtLength(this->m_thinPath.length() - row * this->m_cellLength);
		QPointF centerPt = this->m_thinPath.pointAtPercent(atPercent);
		qreal angle = qDegreesToRadians(this->m_thinPath.angleAtPercent(atPercent));

		//left side
		QPointF lCenterPt = centerPt + QPointF((-width / 2 * std::sin(angle)), (-width / 2 * std::cos(angle)));
		QPointF lFrontPt = lCenterPt + QPointF((arrowLength / 2 * std::cos(angle)), (-arrowLength / 2 * std::sin(angle)));
		QPointF lBackPt = lCenterPt + QPointF((-arrowLength / 2 * std::cos(angle)), (arrowLength / 2 * std::sin(angle)));
		QPointF lPt1 = lBackPt + QPointF((-arrowWidth * std::sin(angle)), (-arrowWidth * std::cos(angle)));
		QPointF lPt2 = lBackPt + QPointF((arrowWidth * std::sin(angle)), (arrowWidth * std::cos(angle)));
		QPainterPath lArrow{ lFrontPt };
		lArrow.lineTo(lPt1);
		lArrow.lineTo(lPt2);
		lArrow.lineTo(lFrontPt);
		this->m_pointyBits.push_back(this->path().intersected(lArrow));

		//right side
		QPointF rCenterPt = centerPt + QPointF((width / 2 * std::sin(angle)), (width / 2 * std::cos(angle)));
		QPointF rFrontPt = rCenterPt + QPointF((arrowLength / 2 * std::cos(angle)), (-arrowLength / 2 * std::sin(angle)));
		QPointF rBackPt = rCenterPt + QPointF((-arrowLength / 2 * std::cos(angle)), (arrowLength / 2 * std::sin(angle)));
		QPointF rPt1 = rBackPt + QPointF((-arrowWidth * std::sin(angle)), (-arrowWidth * std::cos(angle)));
		QPointF rPt2 = rBackPt + QPointF((arrowWidth * std::sin(angle)), (arrowWidth * std::cos(angle)));
		QPainterPath rArrow{ rFrontPt };
		rArrow.lineTo(rPt1);
		rArrow.lineTo(rPt2);
		rArrow.lineTo(rFrontPt);
		this->m_pointyBits.push_back(this->path().intersected(rArrow));
	}
}
