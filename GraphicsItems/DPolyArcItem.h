#pragma once

// Qt lib
#include <QObject>
#include <QGraphicsPathItem>

class DCellItem;

class DPolyArcItem : public QGraphicsPathItem {

private:

	const static double arcBuffer;
	const static double arrowLength;
	const static double arrowWidth;

	const static QColor m_color;
	const static QColor m_arrowColor;
	const static QColor m_selectedColor;

	const int m_arcId;

	double m_cellLength = 0;
	int m_numOfLanes = 0;
	int m_numOfRows = 0;

	QPainterPath m_thinPath;
	QList<QPainterPath> m_pointyBits;

public:
	DPolyArcItem(int arcId, double cellLength = 1, int numOfLanes = 1, int numOfRows = 1, const QList<QPointF>& ptList = {});
	~DPolyArcItem();

	// --- --- --- --- --- Editing Functions --- --- --- --- ---

	void updateSize(double cellLength, int numOfLanes, int numOfRows);

	void updatePos(QList<QPointF> ptList);

	void setCellItemRotPos(const QList<QList<DCellItem*>>& cellItems) const;

	void setCellItemRotPos(int row, const QList<DCellItem*>& cellItems) const;

	// --- --- --- --- --- Getter --- --- --- --- ---

	int getArcId() const;

	int getRowClosestToPt(QPointF scenePt) const;

	double getRowRot(int row) const;

	QPointF getRowCenterPt(int row) const;

	// --- --- --- --- --- Override QGraphicsItem --- --- --- --- ---

	int type() const override;

	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0) override;

private:

	// --- --- --- --- --- Private Util --- --- --- --- ---

	void calDrawingBits();
};
