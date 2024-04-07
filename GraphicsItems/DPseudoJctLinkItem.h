#pragma once

// Qt lib
#include <QObject>
#include <QGraphicsPathItem>

#include "DJctLinkItem.h"

class DPseudoJctLinkItem : public QGraphicsPathItem {

private:

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

	const static QColor m_color;
	const static QColor m_selectedColor;
	const static int m_arrowSize;

	bool m_useCustomColor = false;
	QColor m_customColor = m_color;

	QPainterPath BezierCurve_path;

	// type the curved line used in this connector junction link
	enum TypeOfcurve { Zigzag, UTurn, Bezier};
	TypeOfcurve TypeOfcurveLine;

	// --- --- --- --- --- inner customized --- --- --- --- ---
	void ComputePath();
	void ComputeArrow();

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---
	DPseudoJctLinkItem(QPointF startPt, double startAngle, QPointF endPt, double endAngle);
	~DPseudoJctLinkItem();

	void setStartPt(QPointF startPt);
	void setEndPt(QPointF endPt);

	// --- --- --- --- --- Getter --- --- --- --- ---
	double getLength();

	// --- --- --- --- --- Editing Functions --- --- --- --- ---
	void setUseCustomColor(bool isUse, QColor color = {});

	// --- --- --- --- --- Override QGraphicsItem --- --- --- --- ---
	int type() const override;
	//QRectF boundingRect() const override;   // new boundingRect will be updated from shape() if no customized change on boundingrect
	QPainterPath shape() const override;
	

protected:
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;


};
