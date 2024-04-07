#include "DPseudoJctLinkItem.h"

// std lib
#include <cmath>

// Qt lib
#include <QPen>
#include <QtMath>
#include <QPainter>

// DISCO_GUI lib
#include "DGraphicsScene.h"


// --- --- --- --- --- Init Const Static Stuff --- --- --- --- ---

const QColor DPseudoJctLinkItem::m_color = Qt::GlobalColor::black;
const QColor DPseudoJctLinkItem::m_selectedColor = Qt::GlobalColor::red;
const int DPseudoJctLinkItem::m_arrowSize = 1;
const double DPseudoJctLinkItem::AngleBoundaryRange = 8;
const double DPseudoJctLinkItem::Curvedpercent = 0.80;

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DPseudoJctLinkItem::DPseudoJctLinkItem(QPointF startPt, double startAngle, QPointF endPt, double endAngle) 
    : m_startPt(startPt), m_startAngle(startAngle), m_endPt(endPt), m_endAngle(endAngle){
    // line not yet ready
    //this->setLine({ this->m_startPt, this->m_endPt });
   
    //set default z-value (layer)
    this->setZValue(DGraphicsScene::ZValue_GraphicItemLayer::PseudoJLinkItem);
    //Flags
    this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsSelectable);
    this->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIsMovable, false);
    //Style
    this->setPen(QPen(this->m_color, 0.5, Qt::SolidLine, Qt::FlatCap, Qt::PenJoinStyle::MiterJoin));
	
    // for any subsequent repaint requests, the Graphics View framework will redraw from the cache
    this->setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    // formation of the BezierCurve
    this->BezierCurve_path = QPainterPath();
    this->BezierCurve_path.moveTo(startPt);

    // prepare the angle difference between start and end point
    if ((m_startAngle >= 0 && m_endAngle >= 0 ) || (m_startAngle < 0 && m_endAngle < 0)){
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
            this->delta_Angle = 360- (startAngle_plus_endAngle - 360);
        }
    }
    


    ComputePath();

    // Sets the item's path to be the given path.
    this->setPath(this->BezierCurve_path);

}

DPseudoJctLinkItem::~DPseudoJctLinkItem(){
}

// --- --- --- --- --- Getter --- --- --- --- ---

double DPseudoJctLinkItem::getLength() {
    return this->BezierCurve_path.length();
}

// --- --- --- --- --- Editing Functions --- --- --- --- ---
void DPseudoJctLinkItem::setUseCustomColor(bool isUse, QColor color) {
    this->m_useCustomColor = isUse;
    this->m_customColor = std::move(color);
}

// --- --- --- --- --- inner customized --- --- --- --- ---
void DPseudoJctLinkItem::ComputePath() {
    if (this->delta_Angle >= 0 && this->delta_Angle <= this->AngleBoundaryRange*2) {
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
        if (std::abs(std::abs(this->m_startAngle) - vertex_angle) <90){
        }
        else {
            QLineF tmp_midline = QLineF(midBasePt, midBasePt + QPointF(tmp_vertexline.length(),0));  //avoid failure on creating tmp line
            tmp_midline.setAngle(vertex_angle-180);
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

    // fill in m_arrowHeadPolygon
    ComputeArrow();
}


void DPseudoJctLinkItem::ComputeArrow() {
    // finding coords of arrow polygon shape point 
    // avoid it keep recalculated whereas will increase the computational heap
    // once arrowhead is inited then the  arrowhead list is not empty anymore (composed as QPolygonF(is kind of list))
    QLineF ArrowEndLine = QLineF(this->m_endPt, this->BC_endPt3);

    // start drawing the arrowhead
    double angle = std::acos(ArrowEndLine.dx() / ArrowEndLine.length());

    if (ArrowEndLine.dy() >= 0) {
        angle = (M_PI * 2.0) - angle;
    }

    double arrowx1, arrowy1, arrowx2, arrowy2;
    if (this->TypeOfcurveLine == TypeOfcurve::UTurn) {
        // UTurn, reverse the arrow direction
        arrowx1 = this->m_endPt.x() - std::sin(angle + M_PI / 3.0) * this->m_arrowSize;
        arrowy1 = this->m_endPt.y() - std::cos(angle + M_PI / 3.0) * this->m_arrowSize;

        arrowx2 = this->m_endPt.x() - std::sin(angle + M_PI - M_PI / 3.0) * this->m_arrowSize;
        arrowy2 = this->m_endPt.y() - std::cos(angle + M_PI - M_PI / 3.0) * this->m_arrowSize;
    }
    else {
        // Zigzag and beizer curve, normal arrow direction
        arrowx1 = this->m_endPt.x() + std::sin(angle + M_PI / 3.0) * this->m_arrowSize;
        arrowy1 = this->m_endPt.y() + std::cos(angle + M_PI / 3.0) * this->m_arrowSize;

        arrowx2 = this->m_endPt.x() + std::sin(angle + M_PI - M_PI / 3.0) * this->m_arrowSize;
        arrowy2 = this->m_endPt.y() + std::cos(angle + M_PI - M_PI / 3.0) * this->m_arrowSize;
    }

    QPointF arrowP1 = QPointF(arrowx1, arrowy1);
    QPointF arrowP2 = QPointF(arrowx2, arrowy2);

    this->m_arrowHeadPolygon.clear();
    this->m_arrowHeadPolygon << this->m_endPt << arrowP1 << arrowP2;
}

void DPseudoJctLinkItem::setStartPt(QPointF startPt) {
    this->prepareGeometryChange();
    this->m_startPt = startPt;

    // only starting pt is chnaged
    this->BezierCurve_path.moveTo(startPt);
    
    // Sets the item's path to be the given path.
    this->setPath(this->BezierCurve_path);
}

void DPseudoJctLinkItem::setEndPt(QPointF endPt) {
    this->prepareGeometryChange();
    this->m_endPt = endPt;

    // all pt r changed except starting pt
    ComputePath();

    // Sets the item's path to be the given path.
    this->setPath(this->BezierCurve_path);

    this->update();
}


// --- --- --- --- --- Override QGraphicsItem --- --- --- --- ---

int DPseudoJctLinkItem::type() const {
    return UserType + DGraphicsScene::ZValue_GraphicItemLayer::PseudoJLinkItem;
}

//// new boundingRect will be updated from shape() if no customized change on boundingrect
//QRectF DPseudoJctLinkItem::boundingRect() const {
//    return this->shape().boundingRect();    // this will make it become rectangular bounding again
//}

QPainterPath DPseudoJctLinkItem::shape() const {
    QPainterPath path = QGraphicsPathItem::shape();
    path.addPolygon(m_arrowHeadPolygon);

    QPainterPathStroker PathStroker{};
    PathStroker.setWidth(1);
    PathStroker.setCapStyle(Qt::RoundCap);
    path = PathStroker.createStroke(path);

    return path;
}

void DPseudoJctLinkItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    auto thisColor = (this->m_useCustomColor ? this->m_customColor : (this->isSelected() ? this->m_selectedColor : this->m_color));

    QPen myPen = pen();
    myPen.setColor(thisColor);
    
    painter->setPen(myPen);
   
    // cancel the saw shape in the line ; turn on HD mode
    painter->setRenderHints(QPainter::Antialiasing, true);

    // brush path
    painter->drawPath(this->BezierCurve_path);

    // brush arrow
    painter->setBrush(thisColor);

    // draw arrow
    painter->drawPolygon(this->m_arrowHeadPolygon, Qt::OddEvenFill);

    //// this is to draw out the path_outlines(which is also new bounding of the current path)
    //myPen.setWidth(1);
    //myPen.setColor(thisColor);
    //myPen.setStyle(Qt::DashDotLine);
    //painter->setBrush(Qt::NoBrush);
    //painter->drawPath(this->shape());
}


