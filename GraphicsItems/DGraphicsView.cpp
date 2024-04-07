#include "DGraphicsView.h"

#include <QMouseEvent>

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DGraphicsView::DGraphicsView(QGraphicsScene* scene, QWidget* parent) : QGraphicsView(scene, parent) {
	//default settings
	this->setDragMode(this->temp_dragMode);
	this->setMouseTracking(true);

	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

DGraphicsView::~DGraphicsView() {
}

// --- --- --- --- --- Setter --- --- --- --- ---

void DGraphicsView::setViewMoveable(bool allowViewMove) {
	this->mStat_allowViewMove = allowViewMove;
	if (allowViewMove)
		this->setDragMode(this->temp_dragMode);
	else {
		this->temp_dragMode = this->dragMode();
		this->setDragMode(QGraphicsView::DragMode::NoDrag);
	}
}

void DGraphicsView::setMiddleClickPanMode(bool allowMiddleClickPan) {
	this->mStat_allowMiddleClickPan = allowMiddleClickPan;
}

void DGraphicsView::setZoomMode(bool allowZoom) {
	this->mStat_allowZoom = allowZoom;
}

// --- --- --- --- --- Events Handling --- --- --- --- ---

void DGraphicsView::mousePressEvent(QMouseEvent* event) {
	//Allow middle click draging
	if (this->mStat_allowViewMove && this->mStat_allowMiddleClickPan && event->button() == Qt::MouseButton::MiddleButton) {
		//config graphics view
		this->temp_dragMode = this->dragMode();
		this->setDragMode(QGraphicsView::DragMode::ScrollHandDrag);
		this->setInteractive(false);
		//create dummy left click drag event
		QMouseEvent* pressEvent = new QMouseEvent(QEvent::GraphicsSceneMousePress,
			event->pos(), Qt::MouseButton::LeftButton,
			Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier);
		this->QGraphicsView::mousePressEvent(pressEvent);
		//kill the original event
		event->accept();
	}
	else {
		//otherwise default behaviour
		this->QGraphicsView::mousePressEvent(event);
	}
}

void DGraphicsView::mouseReleaseEvent(QMouseEvent* event) {
	//Allow middle click draging
	if (this->mStat_allowViewMove && this->mStat_allowMiddleClickPan && event->button() == Qt::MouseButton::MiddleButton) {
		//create dummy left click drag event
		QMouseEvent* releaseEvent = new QMouseEvent(QEvent::GraphicsSceneMouseRelease,
			event->pos(), Qt::MouseButton::LeftButton,
			Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier);
		this->QGraphicsView::mouseReleaseEvent(releaseEvent);
		//config graphics view back to orginal state
		this->setDragMode(this->temp_dragMode);
		this->setInteractive(true);
		//kill the original event
		event->accept();
	}
	else
		this->QGraphicsView::mouseReleaseEvent(event);
}

void DGraphicsView::wheelEvent(QWheelEvent* event) {
	if (this->mStat_allowViewMove && this->mStat_allowZoom) {
		if (event->angleDelta().y() > 0) {
			this->scale(1.2, 1.2);
		}
		else {
			this->scale(1 / 1.2, 1 / 1.2);
		}
	}
	event->accept();
}

void DGraphicsView::resizeEvent(QResizeEvent* event) {
	emit this->graphicsViewResized();
	QGraphicsView::resizeEvent(event);
}
