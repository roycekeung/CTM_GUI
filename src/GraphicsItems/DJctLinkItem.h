#pragma once

#include <vector>

// Qt lib
#include <QObject>
#include <QGraphicsPathItem>

// DISCO_GUI lib
#include "DGraphicsScene.h"

class DPseudoJctLinkItem;

class DJctLinkItem : public QGraphicsPathItem {

private:
	int m_arcId;
	int m_jLinkId;

	QPointF m_startPt;
	QPointF m_endPt;
	double m_startAngle;
	double m_endAngle;

	QPointF intersectPt;
	QPointF BC_startPt2;
	QPointF BC_endPt3;

	double delta_Angle;  // m_endPt m_startPt angle diff, abs
	const static double AngleBoundaryRange;
	const static double Curvedpercent;



	QPolygonF m_arrowHeadPolygon;
	std::vector<QPainterPath> ArrowsHeadPathInBaseArc;

	const static QColor m_color;
	const static QColor m_selectedColor;
	const static QColor m_arrowColor;

	const static int m_arrowSize;
	const static int arrowLength;
	const static double arcBuffer;

	// center BezierCurve line
	QPainterPath BezierCurve_path;   
	// formation of the offset of center BezierCurve in which is uesed for Junction Link Base Arc
	QPainterPath BezierCurveBaseArc_Path;

	int m_numOfLanes;

	// type the curved line used in this connector junction link
	enum TypeOfcurve { Zigzag, UTurn, Bezier };
	TypeOfcurve TypeOfcurveLine;

	// --- --- --- --- --- inner customized --- --- --- --- ---
	void ComputePath();
	void ComputeOffsetOfCenterPath();
	void ComputeHalfArrowsForCell(QPointF CellFrontPosOnBaseArc, QPointF CellBackPosOnBaseArc);


public:
	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---
	DJctLinkItem(int arcId, int jLinkId, QPointF startPt = QPointF{ 0,0 }, double startAngle = 0, QPointF endPt = QPointF{ 0,0 }, double endAngle = 0, int numOfLanes = 1);
	~DJctLinkItem();

	// --- --- --- --- --- Setting --- --- --- --- ---
	void setStartPt(QPointF startPt, double startAngle);
	void setEndPt(QPointF endPt, double endAngle);
	void setNumOfLanes(int numOfLanes);
	void updateSizeAndPos(QPointF startPt, double startAngle, QPointF endPt, double endAngle, int numOfLanes);
	// used in DgraphicScene for inserting the cells into the base arc
	void setItemPosAndRotation(std::vector<QGraphicsItem*> items);

	// --- --- --- --- --- Getter --- --- --- --- ---
	int getArcId();
	int getJctLinkId();

	// --- --- --- --- --- Override QGraphicsItem --- --- --- --- ---
	int type() const override;
	//QRectF boundingRect() const override;   // new boundingRect will be updated from shape() if no customized change on boundingrect
	//QPainterPath shape() const override;


protected:
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;


};
