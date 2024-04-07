#pragma once

#include <Qobject>
#include <QPen>
#include <QBrush>
#include <QGraphicsRectItem>

/**
 
 Rotation -> drawn such that 0 degrees is pointing to positive x direction
		  -> so the width is vf, and hight is laneWidth
*/
class DCellItem : public QObject, public QGraphicsRectItem {
	Q_OBJECT

private:

	const int m_cellId;
	int m_numOfLanes;

	const static QColor m_color;
	const static QColor m_selectedColor;
	const static QColor m_lineColor;

	bool m_useCustomColor = false;
	QColor m_customColor = m_color;

	QPen m_pen;
	QBrush m_brush{ Qt::BrushStyle::SolidPattern };

	bool m_useCustomText = false;
	QString m_customText;
	bool m_textCacheIsValid = false;
	qreal m_ptSize;
	QTransform m_textTransform;

public:

	const static double cellWidthBuffer;

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DCellItem(int cellId, int numOfLanes = 1, double length = 1);
	~DCellItem();

	// --- --- --- --- --- Getters --- --- --- --- ---

	int getCellId();

	int getNumOfLanes();

	//Point on scene where exiting connectors starts at
	QPointF getConnectorExitPt();
	//Point on scene where entering connectors ends at
	QPointF getConnectorEnterPt();

	// --- --- --- --- --- Editing Functions --- --- --- --- ---

	void updateSize(int numOfLanes, double length);

	void setUseCustomColor(bool isUse, QColor color = {});

	void setUseText(bool isUse, QString&& str = {});

	// --- --- --- --- --- Override QGraphicsItem --- --- --- --- ---

	int type() const override;

	virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0) override;

};
