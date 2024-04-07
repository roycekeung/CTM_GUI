#pragma once

//Qt lib
#include <QtCore/QObject>
#include <QPen>
#include <QBrush>
#include <QGraphicsRectItem>

/**

 Rotation -> drawn such that 0 degrees is pointing to positive x direction
		  -> so the width is vf, and hight is laneWidth
*/

class DDemandSinkCellItem : public QObject, public QGraphicsRectItem{
	Q_OBJECT
public:
	// --- --- --- --- --- Enum cell type --- --- --- --- ---
	enum CellType { Demand, Sink };

private:	
	// --- --- --- --- --- storing of Enum type and cell id--- --- --- --- ---
	CellType typeOfCell = CellType::Demand;
	int m_cellId;

	// --- --- --- --- --- graphic param --- --- --- --- ---
	const static QColor m_originalCellcolor;
	const static QColor m_DmdCellcolor;
	const static QColor m_SnkCellcolor;
	const static QColor m_selectedColor;
	const static QColor m_lineColor;

	const static QColor m_arrowColor;

	QColor m_Color = m_originalCellcolor;
	bool m_useCustomColor = false;
	QColor m_customColor = m_originalCellcolor;
	
	bool m_useCustomText = false;
	QString m_customText;
	bool m_textCacheIsValid = false;
	qreal m_ptSize;
	QTransform m_textTransform;

	QPen m_pen;
	QBrush m_brush{ Qt::BrushStyle::SolidPattern };

	const static double cellWidthBuffer;
	const static double arrowLength;
	const static double arrowWidth;

	int m_numOfLanes;
	bool m_isResizing;
	bool m_isRotating;
	bool m_isMovableRotatable = false;

	double offset;
	QRectF topLeftRect;
	QRectF topRightRect;
	QRectF bottomLeftRect;
	QRectF bottomRightRect;
	QRectF RotateRect;
	enum mousePressArea { topleft , topRight, bottomLeft, bottomRight, rotate ,None};
	mousePressArea mouseCurrentArea;

	double m_scale = 1;

	// --- --- --- --- --- inner Functions --- --- --- --- ---
	inline qreal GetDegreeAngle(QVector2D vector2d) const;
	void updateResizeHandles();
	void updateRotateHandles();

	// --- --- --- --- --- filter Functions --- --- --- --- ---
	bool isInResizeArea(const QPointF& pos);
	bool isInRotateArea(const QPointF& pos);

public:
	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---
	DDemandSinkCellItem(int cellId = 0 , int numOfLanes = 1);
	~DDemandSinkCellItem();

	// --- --- --- --- --- Getters --- --- --- --- ---
	int getCellId();
	//Point on scene where exiting connectors starts at
	QPointF getConnectorExitPt();
	//Point on scene where entering connectors ends at
	QPointF getConnectorEnterPt();
	DDemandSinkCellItem::CellType getTypeOfCell();
	int getnumOfLanes();
	double getScale();

	// --- --- --- --- --- Editing Functions --- --- --- --- ---
	void updateSize(int numOfLanes);

	// --- --- --- --- --- Setting --- --- --- --- ---
	void setTypeOfCell(CellType type);
	void setCellID(int CellID);
	void setMovableRotatable(bool MovableAndRotatable);
	void setUseCustomColor(bool isUse, QColor color = {});
	void setUseText(bool isUse, QString&& str = {});
	void setnumOfLanes(int numOfLanes);
	void setScale(double ratio);

	// --- --- --- --- --- Override QGraphicsItem --- --- --- --- ---
	int type() const override;
	virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0) override;
	QRectF boundingRect() const override ;

	void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
	void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;
	void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

signals:
	// --- --- --- --- --- Customized Signal --- --- --- --- ---
	void ShapePosChange();
};

