#pragma once

#include <QGraphicsView>

/**
 * Mainly needed to impl middle mouse click dragging
 */
class DGraphicsView : public QGraphicsView {
	Q_OBJECT

private:

	// --- --- --- --- --- Display States --- --- --- --- ---
	bool mStat_allowViewMove = true;			//iff false locks view from any movement of user input
	bool mStat_allowMiddleClickPan = true;		//iff true enable middle click dragging
	bool mStat_allowZoom = true;				//iff true enable zooming
	QGraphicsView::DragMode temp_dragMode = QGraphicsView::DragMode::RubberBandDrag;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DGraphicsView(QGraphicsScene* scene, QWidget* parent);

	~DGraphicsView();

signals:

	void graphicsViewResized();

public slots:

	// --- --- --- --- --- Setter --- --- --- --- ---

	/**
	iff false locks view from any movement of user input
	sets drag mode to noDrag
	*/
	void setViewMoveable(bool allowViewMove);

	/**
	iff true enable middle click dragging
	*/
	void setMiddleClickPanMode(bool allowMiddleClickPan);

	/**
	iff true enable zooming
	*/
	void setZoomMode(bool allowZoom);

protected:

	// --- --- --- --- --- Events Handling --- --- --- --- ---
	
	/**
	Reimplemented to allow middle click dragging
	*/
	virtual void mousePressEvent(QMouseEvent* event) override;

	/**
	Reimplemented to allow middle click dragging
	*/
	virtual void mouseReleaseEvent(QMouseEvent* event) override;

	/**
	Reimplemented to defult to scroll zooming
	*/
	virtual void wheelEvent(QWheelEvent* event) override;

	/**
	* Reimplemented to emit signal
	*/
	virtual void resizeEvent(QResizeEvent* event) override;

};
