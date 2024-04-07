#include "DJctLinkItem.h"

// std lib
#include <cmath>

// Qt lib
#include <QPen>
#include <QtMath>
#include <QPainter>
#include <qgraphicsitem.h>

#include "DPseudoJctLinkItem.h"

// --- --- --- --- --- Init Const Static Stuff --- --- --- --- ---

const QColor DJctLinkItem::m_color = QColor{ 97, 97, 97 };
const QColor DJctLinkItem::m_selectedColor = Qt::GlobalColor::red;
const QColor DJctLinkItem::m_arrowColor = QColor{ 251, 255, 0 };
const int DJctLinkItem::m_arrowSize = 1;
const int DJctLinkItem::arrowLength = 2;
const double DJctLinkItem::arcBuffer = 0.5;

const double DJctLinkItem::AngleBoundaryRange = 8;
const double DJctLinkItem::Curvedpercent = 0.80;

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DJctLinkItem::DJctLinkItem(int arcId, int jLinkId, QPointF startPt, double startAngle, QPointF endPt, double endAngle, int numOfLanes)
    : m_arcId(arcId), m_jLinkId(jLinkId), m_startPt(startPt), m_startAngle(startAngle), m_endPt(endPt), m_endAngle(endAngle), m_numOfLanes(numOfLanes){

    //set default z-value
    this->setZValue(DGraphicsScene::ZValue_GraphicItemLayer::JLinkItem);  // graphical layout put it at the bottom of the cells

    //Flags
    this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable);
    this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, false);
    //Style
    this->setPen(QPen(this->m_color, 0.5, Qt::SolidLine, Qt::FlatCap, Qt::PenJoinStyle::MiterJoin));

    // for any subsequent repaint requests, the Graphics View framework will redraw from the cache
    this->setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    // prepare the angle difference between start and end point
    if ((m_startAngle >= 0 && m_endAngle >= 0) || (m_startAngle < 0 && m_endAngle < 0)) {
        // means the two object r undergo same clockwise rotation sys
        this->delta_Angle = abs(m_startAngle - m_endAngle);
    }
    else {
        // means the two object r undergo opposite clockwise rotation sys
        double startAngle_plus_endAngle = abs(m_startAngle) + abs(m_endAngle);
        if (startAngle_plus_endAngle <= 180) {
            this->delta_Angle = startAngle_plus_endAngle;
        }
        else if ((startAngle_plus_endAngle > 180) && (startAngle_plus_endAngle <= 360)) {
            this->delta_Angle = abs(abs(m_startAngle) - abs(m_endAngle));
        }
        else if ((startAngle_plus_endAngle > 360) && (startAngle_plus_endAngle <= 540)) {
            this->delta_Angle = startAngle_plus_endAngle - 360;
        }
        else if ((startAngle_plus_endAngle > 540) && (startAngle_plus_endAngle <= 720)) {
            this->delta_Angle = 360 - (startAngle_plus_endAngle - 360);
        }
    }


    // formation of the center BezierCurve
    this->BezierCurve_path = QPainterPath();
    this->BezierCurve_path.moveTo(startPt);

    // formation of the offset of center BezierCurve in which is uesed for Junction Link Base Arc
    this->BezierCurveBaseArc_Path = QPainterPath();

    ComputePath();

    // Sets the item's path to be the given path.
    this->setPath(this->BezierCurveBaseArc_Path);

}

DJctLinkItem::~DJctLinkItem() {
}

// --- --- --- --- --- inner customized --- --- --- --- ---
void DJctLinkItem::ComputePath() {

    if (this->delta_Angle >= 0 && this->delta_Angle <= this->AngleBoundaryRange * 2) {
        // two cells r presumed to be paralleled, pt are on opposite sides, so  Zigzag path
        this->TypeOfcurveLine = TypeOfcurve::Zigzag;

        QLineF tmp_startline = QLineF(this->m_startPt, this->m_startPt + QPointF(1, 0));  //avoid failure on creating tmp line
        // rotate the tmp line, p1 fixed but change with p2
        tmp_startline.setAngle(-m_startAngle);

        QLineF tmp_endline = QLineF(this->m_endPt, this->m_endPt + QPointF(1, 0));  //avoid failure on creating tmp line
        // rotate the tmp line, p1 fixed but change with p2
        tmp_endline.setAngle(-m_endAngle);

        this->intersectPt = QPointF((this->m_startPt + this->m_endPt) / 2);
        QLineF tmp_intersectCutLine = QLineF(this->intersectPt, this->m_startPt);  //avoid failure on creating tmp line
        // rotate the tmp line to perpendicular cut plane
        tmp_intersectCutLine.setAngle(90);

        QPointF* BC_startPt2 = new QPointF();
        QPointF* BC_endPt3 = new QPointF();

        tmp_intersectCutLine.intersects(tmp_startline, BC_startPt2);
        tmp_intersectCutLine.intersects(tmp_endline, BC_endPt3);

        this->BC_startPt2 = *BC_startPt2;
        this->BC_endPt3 = *BC_endPt3;

        // put the BC_startPt2, BC_endPt3 and endPt into the BezierCurve_path
        this->BezierCurve_path.cubicTo(this->BC_startPt2, this->BC_endPt3, this->m_endPt);

    }
    else if ((180 - this->AngleBoundaryRange) <= this->delta_Angle && this->delta_Angle <= (180 + this->AngleBoundaryRange)) {
        // two cells r presumed to be paralleled, pt are on same sides,  so  U-turn path
        this->TypeOfcurveLine = TypeOfcurve::UTurn;

        QLineF tmp_startline = QLineF(this->m_startPt, this->m_startPt + QPointF(1, 0));  //avoid failure on creating tmp line
        // rotate the tmp line, p1 fixed but change with p2
        tmp_startline.setAngle(-m_startAngle + 30 * std::exp(-1 * abs(this->delta_Angle - 180) / this->AngleBoundaryRange));

        QLineF tmp_endline = QLineF(this->m_endPt, this->m_endPt + QPointF(1, 0));  //avoid failure on creating tmp line
        // rotate the tmp line, p1 fixed but change with p2
        tmp_endline.setAngle(-m_endAngle - 30 * std::exp(-1 * abs(this->delta_Angle - 180) / this->AngleBoundaryRange));


        QPointF* intersectPt = new QPointF();
        auto intersectType = tmp_startline.intersects(tmp_endline, intersectPt);
        this->intersectPt = *intersectPt;

        QPointF midBasePt = (this->m_startPt + this->m_endPt) / 2;
        QLineF tmp_vertexline = QLineF(midBasePt, *intersectPt);
        qreal vertex_angle = tmp_vertexline.angle();
        if (std::abs(std::abs(this->m_startAngle) - vertex_angle) < 90) {
        }
        else {
            QLineF tmp_midline = QLineF(midBasePt, midBasePt + QPointF(tmp_vertexline.length(), 0));  //avoid failure on creating tmp line
            tmp_midline.setAngle(vertex_angle - 180);
            this->intersectPt = tmp_midline.p2();
        }


        // put the BC_startPt2, BC_endPt3 and endPt into the BezierCurve_path
        this->BezierCurve_path.quadTo(this->intersectPt, this->m_endPt);

        delete intersectPt;

    }
    else {
        // normal beizer curved line was formed here ,  so  beizer path
        this->TypeOfcurveLine = TypeOfcurve::Bezier;

        QLineF tmp_startline = QLineF(this->m_startPt, this->m_startPt + QPointF(1, 0));  //avoid failure on creating tmp line
        // rotate the tmp line, p1 fixed but change with p2
        tmp_startline.setAngle(-m_startAngle);

        QLineF tmp_endline = QLineF(this->m_endPt, this->m_endPt + QPointF(1, 0));  //avoid failure on creating tmp line
        // rotate the tmp line, p1 fixed but change with p2
        tmp_endline.setAngle(-m_endAngle);

        QPointF* intersectPt = new QPointF();
        auto intersectType = tmp_startline.intersects(tmp_endline, intersectPt);
        this->intersectPt = *intersectPt;

        this->BC_startPt2 = QPointF(this->m_startPt.x() - (this->m_startPt.x() - this->intersectPt.x()) * this->Curvedpercent, this->m_startPt.y() - (this->m_startPt.y() - this->intersectPt.y()) * this->Curvedpercent);
        this->BC_endPt3 = QPointF(this->m_endPt.x() - (this->m_endPt.x() - this->intersectPt.x()) * this->Curvedpercent, this->m_endPt.y() - (this->m_endPt.y() - this->intersectPt.y()) * this->Curvedpercent);

        // put the BC_startPt2, BC_endPt3 and endPt into the BezierCurve_path
        this->BezierCurve_path.cubicTo(this->BC_startPt2, this->BC_endPt3, this->m_endPt);
    }

    //// find in offset path of BezierCurve_path
    ComputeOffsetOfCenterPath();
}

void DJctLinkItem::ComputeOffsetOfCenterPath() {
    // find the outlier path in shape of the offset of Center BezierCurve path
    QPainterPathStroker Offset;
    Offset.setCapStyle(Qt::FlatCap);
    Offset.setWidth(this->m_numOfLanes * DGraphicsScene::laneWidth+ this->arcBuffer *2);

    QPainterPath resultPath = Offset.createStroke(this->BezierCurve_path);
    this->BezierCurveBaseArc_Path = resultPath;
}


void DJctLinkItem::ComputeHalfArrowsForCell(QPointF ArrowFrontPosOnBaseArc, QPointF ArrowBackPosOnBaseArc) {
    // find the path of half arrow
    QLineF ArrowEndLine = QLineF(ArrowFrontPosOnBaseArc, ArrowBackPosOnBaseArc);

    // start drawing the arrowhead
    double angle = std::acos(ArrowEndLine.dx() / ArrowEndLine.length());

    if (ArrowEndLine.dy() >= 0) {
        angle = (M_PI * 2.0) - angle;
    }

    double arrowx1, arrowy1, arrowx2, arrowy2;
    if (this->TypeOfcurveLine == TypeOfcurve::UTurn) {
        // UTurn, reverse the arrow direction
        arrowx1 = ArrowFrontPosOnBaseArc.x() + std::sin(angle + M_PI / 3.0) * this->m_arrowSize;
        arrowy1 = ArrowFrontPosOnBaseArc.y() + std::cos(angle + M_PI / 3.0) * this->m_arrowSize;

        arrowx2 = ArrowFrontPosOnBaseArc.x() + std::sin(angle + M_PI - M_PI / 3.0) * this->m_arrowSize;
        arrowy2 = ArrowFrontPosOnBaseArc.y() + std::cos(angle + M_PI - M_PI / 3.0) * this->m_arrowSize;
    }
    else {
        // Zigzag and beizer curve, normal arrow direction
        arrowx1 = ArrowFrontPosOnBaseArc.x() + std::sin(angle + M_PI / 3.0) * this->m_arrowSize;
        arrowy1 = ArrowFrontPosOnBaseArc.y() + std::cos(angle + M_PI / 3.0) * this->m_arrowSize;

        arrowx2 = ArrowFrontPosOnBaseArc.x() + std::sin(angle + M_PI - M_PI / 3.0) * this->m_arrowSize;
        arrowy2 = ArrowFrontPosOnBaseArc.y() + std::cos(angle + M_PI - M_PI / 3.0) * this->m_arrowSize;
    }

    QPointF arrowP1 = QPointF(arrowx1, arrowy1);
    QPointF arrowP2 = QPointF(arrowx2, arrowy2);

    // create the path in arrow shape on each side(Top and Bottom) of the BezierCurve path according to the pos of cell
    QPainterPath tmp_HalfArrowpath = QPainterPath();
    tmp_HalfArrowpath.moveTo(ArrowFrontPosOnBaseArc);
    tmp_HalfArrowpath.lineTo(arrowP1);  
    tmp_HalfArrowpath.lineTo(arrowP2);
    tmp_HalfArrowpath.lineTo(ArrowFrontPosOnBaseArc);

    QPainterPath tmp_HalfArrowItersectPath = this->BezierCurveBaseArc_Path.intersected(tmp_HalfArrowpath);

    this->ArrowsHeadPathInBaseArc.emplace_back(tmp_HalfArrowItersectPath);

}


void DJctLinkItem::setStartPt(QPointF startPt, double startAngle) {
    this->m_startPt = startPt;
    this->m_startAngle = startAngle;

    this->delta_Angle = abs(startAngle - this->m_endAngle);

    // formation of the center BezierCurve
    this->BezierCurve_path = QPainterPath();
    this->BezierCurve_path.moveTo(startPt);

    // formation of the offset of center BezierCurve in which is uesed for Junction Link Base Arc
    this->BezierCurveBaseArc_Path = QPainterPath();

    ComputePath();

    // Sets the item's path to be the given path.
    this->setPath(this->BezierCurveBaseArc_Path);

    //assume always need to update everything
    this->prepareGeometryChange();
    this->update();
}

void DJctLinkItem::setEndPt(QPointF endPt, double endAngle) {
    this->m_endPt = endPt;
    this->m_endAngle = endAngle;

    this->delta_Angle = abs(this->m_startAngle - endAngle);

    // formation of the center BezierCurve
    this->BezierCurve_path = QPainterPath();
    this->BezierCurve_path.moveTo(this->m_startPt);

    // formation of the offset of center BezierCurve in which is uesed for Junction Link Base Arc
    this->BezierCurveBaseArc_Path = QPainterPath();

    ComputePath();

    // Sets the item's path to be the given path.
    this->setPath(this->BezierCurveBaseArc_Path);

    //assume always need to update everything
    this->prepareGeometryChange();
    this->update();
}

void DJctLinkItem::setNumOfLanes(int numOfLanes) {
    this->m_numOfLanes = numOfLanes;
    // this offset width will be updated by the newly input numOfLanes
    ComputeOffsetOfCenterPath();

    //assume always need to update everything
    this->prepareGeometryChange();
    this->update();
}


void DJctLinkItem::updateSizeAndPos(QPointF startPt, double startAngle, QPointF endPt, double endAngle, int numOfLanes) {

    this->m_startPt = startPt;
    this->m_startAngle = startAngle;
    this->BezierCurve_path.moveTo(startPt);

    this->m_endPt = endPt;
    this->m_endAngle = endAngle;

    this->m_numOfLanes = numOfLanes;

    this->delta_Angle = abs(startAngle - endAngle);

    // formation of the center BezierCurve
    this->BezierCurve_path = QPainterPath();
    this->BezierCurve_path.moveTo(this->m_startPt);

    // formation of the offset of center BezierCurve in which is uesed for Junction Link Base Arc
    this->BezierCurveBaseArc_Path = QPainterPath();

    ComputePath();

    // Sets the item's path to be the given path.
    this->setPath(this->BezierCurveBaseArc_Path);

    //assume always need to update everything
    this->prepareGeometryChange();
    this->update();
}

void DJctLinkItem::setItemPosAndRotation(std::vector<QGraphicsItem*> items) {
    // Bezier Curve Base Arc path inf pre cal
    double OffsetAverageLengthOfOneCurvedSide = ((this->BezierCurveBaseArc_Path.length() - (this->m_numOfLanes * DGraphicsScene::laneWidth + this->arcBuffer * 2) * 2) / 2);
    double FulloffsetCurvedLength = this->BezierCurveBaseArc_Path.length(); // dont know how long
    double OffsetCurvedpathSplitLength = ((this->BezierCurveBaseArc_Path.length()- (this->m_numOfLanes * DGraphicsScene::laneWidth + this->arcBuffer * 2)*2)
        /2) / (items.size() + 1);

    // Center Bezier Curve path inf pre cal
    double FullCenterPathLength = this->BezierCurve_path.length();
    double centerCurvedPathSplitLength = FullCenterPathLength / (items.size() + 1);

    // cell inf pre def val
    double currentSplitRatio;
    qreal RationPtRotation;

    // half arrow inf pre def val
    double HalfTopArrowFrontRatio, HalfTopArrowEndRatio, HalfBotArrowFrontRatio, HalfBotArrowEndRatio;
    QPointF HalfTopArrowFrontPt, HalfTopArrowEndPt, HalfBotArrowFrontPt, HalfBotArrowEndPt;

    int count =1;  // cell item loop count
    for (auto& item : items) {
        // --- --- --- --- --- cal and set the pos and rotation by length determination method --- --- --- --- ---
        // the cell start from the end of this arc base, list out in order along the way to start pt
        currentSplitRatio = this->BezierCurve_path.percentAtLength(FullCenterPathLength - centerCurvedPathSplitLength * count);
        qreal RationPtRotation = this->BezierCurve_path.angleAtPercent(currentSplitRatio);

        // set item pos and rotation 
        item->setPos(this->BezierCurve_path.pointAtPercent(currentSplitRatio));
        item->setRotation(-RationPtRotation);


        // --- --- --- --- --- draw half arrow --- --- --- --- ---
        // find Half Top Arrow pos
        HalfTopArrowFrontRatio = this->BezierCurveBaseArc_Path.percentAtLength(
            OffsetAverageLengthOfOneCurvedSide - OffsetCurvedpathSplitLength * count + this->arrowLength/2);
        HalfTopArrowFrontPt = this->BezierCurveBaseArc_Path.pointAtPercent(HalfTopArrowFrontRatio);

         HalfTopArrowEndRatio = this->BezierCurveBaseArc_Path.percentAtLength(
            OffsetAverageLengthOfOneCurvedSide - OffsetCurvedpathSplitLength * count - this->arrowLength/2);
        HalfTopArrowEndPt = this->BezierCurveBaseArc_Path.pointAtPercent(HalfTopArrowEndRatio);

        ComputeHalfArrowsForCell(HalfTopArrowFrontPt, HalfTopArrowEndPt);

        // find Half Bottom Arrow pos
        HalfBotArrowFrontRatio = this->BezierCurveBaseArc_Path.percentAtLength(
            OffsetAverageLengthOfOneCurvedSide + (this->m_numOfLanes * DGraphicsScene::laneWidth + this->arcBuffer * 2 )
            + OffsetCurvedpathSplitLength * count - this->arrowLength / 2);
        HalfBotArrowFrontPt = this->BezierCurveBaseArc_Path.pointAtPercent(HalfBotArrowFrontRatio);

        HalfBotArrowEndRatio = this->BezierCurveBaseArc_Path.percentAtLength(
            OffsetAverageLengthOfOneCurvedSide + (this->m_numOfLanes * DGraphicsScene::laneWidth + this->arcBuffer * 2) 
            + OffsetCurvedpathSplitLength * count + this->arrowLength / 2);
        HalfBotArrowEndPt = this->BezierCurveBaseArc_Path.pointAtPercent(HalfBotArrowEndRatio);

        ComputeHalfArrowsForCell(HalfBotArrowFrontPt, HalfBotArrowEndPt);
    
        // update the count of cell had been setup
        count++;
    }

    //assume always need to update everything
    this->prepareGeometryChange();
    this->update();
}

int DJctLinkItem::getArcId() {
    return this->m_arcId;
}

int DJctLinkItem::getJctLinkId() {
    return this->m_jLinkId;
}


// --- --- --- --- --- Override QGraphicsItem --- --- --- --- ---
int DJctLinkItem::type() const {
    return UserType + DGraphicsScene::ZValue_GraphicItemLayer::JLinkItem;
}

//// new boundingRect will be updated from shape() if no customized change on boundingrect
//QRectF DJctLinkItem::boundingRect() const {
//    return this->shape().boundingRect();    // this will make it become rectangular bounding again
//}

//QPainterPath DJctLinkItem::shape() const {
//    QPainterPath path = QGraphicsPathItem::shape();
//
//    QPainterPathStroker PathStroker{};
//    PathStroker.setWidth(1);
//    PathStroker.setCapStyle(Qt::RoundCap);
//    path = PathStroker.createStroke(path);
//
//    return path;
//}

void DJctLinkItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    QPen myPen = pen();
    myPen.setColor(this->isSelected() ? this->m_selectedColor : this->m_color);

    painter->setPen(myPen);

    // cancel the saw shape in the line ; turn on HD mode
    painter->setRenderHints(QPainter::Antialiasing, true);

    //// brush offset path of BezierCurve_path
    painter->setBrush(this->isSelected() ? this->m_selectedColor : this->m_color);
    // draw offset path of BezierCurve_path
    painter->drawPath(this->BezierCurveBaseArc_Path);

    //// tmp test on the bounding rect area, visually debug
    //painter->setPen(Qt::GlobalColor::blue);
    //painter->setBrush(Qt::NoBrush);
    //painter->drawRect(this->BezierCurveBaseArc_Path.boundingRect());

    //// draw half arrow
    painter->setPen(this->m_arrowColor);
    painter->setPen(QPen(this->m_arrowColor, this->arcBuffer, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
    painter->setBrush(this->m_arrowColor);
    for (auto HalfArrowPath : ArrowsHeadPathInBaseArc) {

        painter->drawPath(HalfArrowPath);

    }

}



