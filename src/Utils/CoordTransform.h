#pragma once

#include <QPointF>

#include "VisInfo.h"

/**
 Convenience class to convert coordinates from storage to Qt's display(QGraphics Scene) coord

 Author: JLo
*/
class CoordTransform {
public:
	enum class CoordSystem { WGS84, HK1980 };

private: 

	CoordSystem m_coordSystem = CoordSystem::WGS84;

	//Coords for translating to a "near location" such that error in WGS84 transforms are minimal
	QPointF m_anchor;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	CoordTransform(CoordSystem coordSystem = CoordSystem::WGS84, QPointF anchor = { 0, 0 });

	~CoordTransform();

	// --- --- --- --- --- Setters --- --- --- --- ---

	void setAnchor(QPointF anchor);

	void setCoordSystem(CoordSystem coordSystem);

	// --- --- --- --- --- Getters --- --- --- --- ---

	CoordSystem getCoordSystem() const;

	QPointF getAnchor() const;

	// --- --- --- --- --- Calculations --- --- --- --- ---

	/**
	 Returns distance between the 2 real coordinates in the constructed coord system
	*/
	double calRealDistance(QPointF&& real1, QPointF&& real2) const;
	double calRealDistance(const QPointF& real1, const QPointF& real2) const;

	/**
	 Returns the Qt rotation of the vector from pt1 to pt2
	*/
	double calRotation(QPointF&& pt1, QPointF&& pt2) const;
	double calRotation(const QPointF& pt1, const QPointF& pt2) const;

	/**
	 Returns a new QPointF display(QGraphicsScene) coord of the supplied real coordinates in the constructed coord system
	 Will traslate the result if given delta in display coord
	*/
	QPointF transformToDisplay(double realX, double realY, double displayDeltaX = 0, double displayDeltaY = 0) const;
	QPointF transformToDisplay(QPointF&& real, double displayDeltaX = 0, double displayDeltaY = 0) const;
	QPointF transformToDisplay(const QPointF& real, double displayDeltaX = 0, double displayDeltaY = 0) const;

	/**
	 Returns a new QPointF real coordinates in the constructed coord system of the supplied display(QGraphicsScene) coord
	 Will traslate the input if given delta in display coord
	*/
	QPointF transformToReal(QPointF&& display, double displayDeltaX = 0, double displayDeltaY = 0) const;
	QPointF transformToReal(const QPointF& display, double displayDeltaX = 0, double displayDeltaY = 0) const;

	/**
	 Returns a new QPointF translated real coordinates in the constructed coord system given the deltas in metres
	*/
	QPointF translateReal(double realX, double realY, double realDeltaX, double realDeltaY) const;
	QPointF translateReal(QPointF&& real, double realDeltaX, double realDeltaY) const;
	QPointF translateReal(const QPointF& real, double realDeltaX, double realDeltaY) const;

	// --- --- --- --- --- Utils --- --- --- --- ---

	DISCO2_API::VisInfo::VisInfo_Pt makeVisInfoPt(QPointF&& pt, double orientation = 0) const;
	DISCO2_API::VisInfo::VisInfo_Pt makeVisInfoPt(const QPointF& pt, double orientation = 0) const;

	QPointF makeQPointF(DISCO2_API::VisInfo::VisInfo_Pt&& pt) const;
	QPointF makeQPointF(const DISCO2_API::VisInfo::VisInfo_Pt& pt) const;
};

