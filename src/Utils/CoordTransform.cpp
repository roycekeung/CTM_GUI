#include "CoordTransform.h"

#include <cmath>
#include <qmath.h>

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

CoordTransform::CoordTransform(CoordSystem coordSystem, QPointF anchor) :
	m_coordSystem(coordSystem), m_anchor(anchor) {

}

CoordTransform::~CoordTransform() {

}

// --- --- --- --- --- Setters --- --- --- --- ---

void CoordTransform::setAnchor(QPointF anchor) {
	this->m_anchor = anchor;
}

void CoordTransform::setCoordSystem(CoordSystem coordSystem) {
	this->m_coordSystem = coordSystem;
}

// --- --- --- --- --- Getters --- --- --- --- ---

CoordTransform::CoordSystem CoordTransform::getCoordSystem() const {
	return this->m_coordSystem;
}

QPointF CoordTransform::getAnchor() const {
	return this->m_anchor;
}

// --- --- --- --- --- Calculations --- --- --- --- ---

double CoordTransform::calRealDistance(QPointF&& real1, QPointF&& real2) const {
	return this->calRealDistance(real1, real2);
}

double CoordTransform::calRealDistance(const QPointF& real1, const QPointF& real2) const {
    using namespace std;

	switch (this->m_coordSystem) {
	case(CoordSystem::WGS84):
    {
        //const
        double bigR = 6371;
        // convert to radians 
        double dLon = (real2.x() - real1.x()) * acos(-1) / 180.0;
        double dLat = (real2.y() - real1.y()) * acos(-1) / 180.0;
        double lat1 = real1.y() * acos(-1) / 180.0;
        double lat2 = real2.y() * acos(-1) / 180.0;

        // apply formula
        double a = pow(sin(dLat / 2), 2) + pow(sin(dLon / 2), 2) * cos(lat1) * cos(lat2);
        return bigR * 2 * asin(sqrt(a));
    }

	case(CoordSystem::HK1980):
	default:
    {
        //just do pythagoras theorem
        using namespace std;
        return sqrt(pow((real2.x() - real1.x()), 2) + pow((real2.y() - real1.y()), 2));
    }
	}
}

double CoordTransform::calRotation(QPointF&& pt1, QPointF&& pt2) const {
    return this->calRotation(pt1, pt2);
}

double CoordTransform::calRotation(const QPointF& pt1, const QPointF& pt2) const {
    return qRadiansToDegrees(-std::atan2(pt2.y() - pt1.y(), pt2.x() - pt1.x()));
}

QPointF CoordTransform::transformToDisplay(double realX, double realY, double displayDeltaX, double displayDeltaY) const {
    return this->transformToDisplay({ realX, realY }, displayDeltaX, displayDeltaY);
}

QPointF CoordTransform::transformToDisplay(QPointF&& real, double displayDeltaX, double displayDeltaY) const {
    return this->transformToDisplay(real, displayDeltaX, displayDeltaY);
}

QPointF CoordTransform::transformToDisplay(const QPointF& real, double displayDeltaX, double displayDeltaY) const {
    using namespace std;

    double realDeltaX = real.x() - this->m_anchor.x();
    double realDeltaY = real.y() - this->m_anchor.y();

    switch (this->m_coordSystem) {
    case(CoordSystem::WGS84):
    {
        //const
        double bigR = 6371;
        double arc = 2 * acos(-1) * bigR / 360.0;
        // degrees delta
        double dLon = (real.x() - this->m_anchor.x());
        double dLat = (real.y() - this->m_anchor.y());
        // convert to radian
        double lat = this->m_anchor.y() * acos(-1) / 180.0;

        return { ((arc * cos(lat) * dLon) + displayDeltaX), (-(arc * dLat) + displayDeltaY) };
    }
    case(CoordSystem::HK1980):
    default:
    {
        //invert the y axis to match qt coord system
        return { (realDeltaX + displayDeltaX), (-realDeltaY + displayDeltaY) };
    }
    }
}

QPointF CoordTransform::transformToReal(QPointF&& display, double displayDeltaX, double displayDeltaY) const {
    return this->transformToReal(display, displayDeltaX, displayDeltaY);
}

QPointF CoordTransform::transformToReal(const QPointF& display, double displayDeltaX, double displayDeltaY) const {
    using namespace std;

    double displayX = display.x() + displayDeltaX;
    double displayY = display.y() + displayDeltaY;

    switch (this->m_coordSystem) {
    case(CoordSystem::WGS84):
    {
        //const
        double bigR = 6371;
        double arc = 2 * acos(-1) * bigR / 360.0;
        // convert to radian
        double lat = this->m_anchor.y() * acos(-1) / 180.0;

        return { (this->m_anchor.x() + (displayX / (arc * cos(lat)))), this->m_anchor.y() - (displayY / arc) };
    }
    case(CoordSystem::HK1980):
    default:
    {
        //invert the y axis to match qt coord system
        return { (this->m_anchor.x() + displayX), (this->m_anchor.y() - displayY) };
    }
    }
}

QPointF CoordTransform::translateReal(double realX, double realY, double realDeltaX, double realDeltaY) const {
    return this->translateReal({ realX, realY }, realDeltaX, realDeltaY);
}

QPointF CoordTransform::translateReal(QPointF&& display, double realDeltaX, double realDeltaY) const {
    return this->translateReal(display, realDeltaX, realDeltaY);
}

QPointF CoordTransform::translateReal(const QPointF& real, double realDeltaX, double realDeltaY) const {
    using namespace std;

    switch (this->m_coordSystem) {
    case(CoordSystem::WGS84):
    {
        //const
        double bigR = 6371;
        double arc = 2 * acos(-1) * bigR / 360.0;
        // convert to radian
        double lat = this->m_anchor.y() * acos(-1) / 180.0;

        return { (real.x() + (realDeltaX / (arc * cos(lat)))), real.y() + (realDeltaY / arc) };
    }
    case(CoordSystem::HK1980):
    default:
    {
        //invert the y axis to match qt coord system
        return { (real.x() + realDeltaX), (real.y() + realDeltaY) };
    }
    }
}

// --- --- --- --- --- Utils --- --- --- --- ---

DISCO2_API::VisInfo::VisInfo_Pt CoordTransform::makeVisInfoPt(QPointF&& pt, double orientation) const {
    return this->makeVisInfoPt(pt, orientation);
}

DISCO2_API::VisInfo::VisInfo_Pt CoordTransform::makeVisInfoPt(const QPointF& pt, double orientation) const {
    return  DISCO2_API::VisInfo::VisInfo_Pt{ pt.x(), pt.y(), orientation };
}

QPointF CoordTransform::makeQPointF(DISCO2_API::VisInfo::VisInfo_Pt&& pt) const {
    return this->makeQPointF(pt);
}

QPointF CoordTransform::makeQPointF(const DISCO2_API::VisInfo::VisInfo_Pt& pt) const {
    return QPointF{ pt.m_X, pt.m_Y};
}
