#include "DDemandSinkCellItem.h"

//DISCO GUI stuff
#include "PanelWidgets/Network/DDemandSinkCellPanel.h"
#include "DDockWidgetHandler.h"
#include "GraphicsItems/DGraphicsScene.h"

//Qt lib
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QMenu>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <qmath.h>   // define M_PI           3.14159265358979323846  /* pi */

constexpr double MAX_ITEM_SIZE = 256;
constexpr double MIN_ITEM_SIZE = 1;
constexpr double filledRatio = 0.08;

const QColor DDemandSinkCellItem::m_originalCellcolor = QColor{ 66, 135, 245 };   // blue
const QColor DDemandSinkCellItem::m_DmdCellcolor = QColor{ 0,0,139 };   // dark blue
const QColor DDemandSinkCellItem::m_SnkCellcolor = QColor{ 144, 196, 255 };   // pale blue
const QColor DDemandSinkCellItem::m_selectedColor = QColor{ 255, 132, 0 };  // orange
const QColor DDemandSinkCellItem::m_lineColor = Qt::GlobalColor::black;

const QColor DDemandSinkCellItem::m_arrowColor = QColor{ 251,255,0 }; // yellow

const double DDemandSinkCellItem::cellWidthBuffer = 0.2;
const double DDemandSinkCellItem::arrowLength = 1.2;
const double DDemandSinkCellItem::arrowWidth = 0.4;




// --- --- --- --- --- inner Functions --- --- --- --- ---
qreal DDemandSinkCellItem::GetDegreeAngle(QVector2D vector2d) const {
	return fmod((atan2((qreal)vector2d.y(), (qreal)vector2d.x()) * M_PI + 360.0), 360.0);
}


void DDemandSinkCellItem::updateResizeHandles() {
	this->offset = std::max(this->cellWidthBuffer * 0.8, this->cellWidthBuffer *this->m_scale * 0.15);

	// this draws correctly on a view with an inverted y axes.i.e.QGraphicsView.scale(1, -1)
	this->topLeftRect = QRectF(this->boundingRect().topLeft().x() ,
		this->boundingRect().topLeft().y() ,
		4 * this->offset, 4 * this->offset);
	this->topRightRect = QRectF(this->boundingRect().topRight().x() - 4 * this->offset, 
		this->boundingRect().topRight().y() , 
		4 * this->offset, 4 * this->offset);
	this->bottomLeftRect = QRectF(this->boundingRect().bottomLeft().x() ,
		this->boundingRect().bottomLeft().y() - 4 * this->offset,
		4 * this->offset, 4 * this->offset);
	this->bottomRightRect = QRectF(this->boundingRect().bottomRight().x() - 4 * this->offset,
		this->boundingRect().bottomRight().y() - 4 * this->offset,
		4 * this->offset, 4 * this->offset);
}

void DDemandSinkCellItem::updateRotateHandles() {
	this->offset = std::max(this->cellWidthBuffer * 0.8, this->cellWidthBuffer * this->m_scale * 0.15);

	// this draws correctly on a view with an inverted y axes.i.e.QGraphicsView.scale(1, -1)
	this->RotateRect = QRectF(this->boundingRect().topRight().x() - 4 * this->offset,
		this->boundingRect().topRight().y() + this->boundingRect().height()/2 - 2 * this->offset,
		4 * this->offset, 4 * this->offset);

}


// --- --- --- --- --- filter Functions --- --- --- --- ---
bool DDemandSinkCellItem::isInResizeArea(const QPointF& pos){
	//// --- --- --- --- --- --- Method 1 : only the right bottom corner could do rescale action --- --- --- --- --- ---
	//double halfWidth = ((this->m_numOfLanes * DGraphicsScene::laneWidth) / 2) - this->cellWidthBuffer;
	//QRectF rect = this->rect();
	//return (-pos.y() < pos.x() - rect.width() + rect.width()/2 * 0.5);


	//// --- --- --- --- --- --- Method 2: by 4 corners --- --- --- --- --- ---
	// Top left corner
	if (this->topLeftRect.contains(pos)) {
		this->mouseCurrentArea = DDemandSinkCellItem::mousePressArea::topleft;
		return true;
	}
	else if (this->topRightRect.contains(pos)) {
		this->mouseCurrentArea = DDemandSinkCellItem::mousePressArea::topRight;
		return true;
	}
	else if (this->bottomLeftRect.contains(pos)) {
		this->mouseCurrentArea = DDemandSinkCellItem::mousePressArea::bottomLeft;
		return true;
	}
	else if (this->bottomRightRect.contains(pos)) {
		this->mouseCurrentArea = DDemandSinkCellItem::mousePressArea::bottomRight;
		return true;
	}
	else {
		this->mouseCurrentArea = DDemandSinkCellItem::mousePressArea::None;
	}	
	return false;

}

bool DDemandSinkCellItem::isInRotateArea(const QPointF& pos) {
	//// define the rotation area here; activate when mouse act on the center region (set on area with width 2/10 of whole rect )
	//if (-(this->rect().width() / 10) <= pos.x() && pos.x() <= (this->rect().width() / 10)
	//	&& -(this->rect().height() / 10) <= pos.y() && pos.y() <= (this->rect().height() / 10)) {
	//	return true;
	//}
	if (this->RotateRect.contains(pos)) {
		this->mouseCurrentArea = DDemandSinkCellItem::mousePressArea::rotate;
		return true;
	}
	else {
		return false;
	}
}


// --- --- --- --- --- Constructor Destructor --- --- --- --- ---
DDemandSinkCellItem::DDemandSinkCellItem(int cellId, int numOfLanes):
	QGraphicsRectItem(nullptr), m_cellId(cellId), m_numOfLanes(numOfLanes), m_isResizing(false), m_isRotating(false) {

	//do the size of this thing
	this->updateSize(numOfLanes);
	this->update();

	//set default z-value (layer)
	this->setZValue(DGraphicsScene::ZValue_GraphicItemLayer::DmdSnkCellItem);
	//Falgs
	this->setFlag(QGraphicsItem::ItemIsMovable, true);
	this->setFlag(QGraphicsItem::ItemIsSelectable, true);
	this->setFlag(QGraphicsItem::ItemIsFocusable, true);   // allows the delivery of key events to QGraphicsItem::keyPressEvent() 
	this->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
	this->setAcceptHoverEvents(true);

	//style
	this->m_pen.setColor(DDemandSinkCellItem::m_lineColor);
	this->m_pen.setCosmetic(true);
	this->m_pen.setWidth(0);

	// default in fixed mode
	setMovableRotatable(false);

	//  independent calls to paint() when refreshing
	setCacheMode(QGraphicsItem::DeviceCoordinateCache);
}

DDemandSinkCellItem::~DDemandSinkCellItem(){

}

// --- --- --- --- --- Getters --- --- --- --- ---
int DDemandSinkCellItem::getCellId() {
	return this->m_cellId;
}

QPointF DDemandSinkCellItem::getConnectorExitPt() {
	qreal heightVf = this->rect().width();
	return this->mapToScene({ (heightVf / 2 - heightVf / 3), 0 });
}

QPointF DDemandSinkCellItem::getConnectorEnterPt() {
	qreal heightVf = this->rect().width();
	return this->mapToScene({ (heightVf / 3 - heightVf / 2), 0 });
}

DDemandSinkCellItem::CellType DDemandSinkCellItem::getTypeOfCell() {
	return this->typeOfCell;
}

int DDemandSinkCellItem::getnumOfLanes() {
	return this->m_numOfLanes;
}

double DDemandSinkCellItem::getScale() {
	return this->m_scale;
}


// --- --- --- --- --- Editing Functions --- --- --- --- ---
void DDemandSinkCellItem::updateSize(int numOfLanes) {
	//notify change first
	this->prepareGeometryChange();
	//geometry change
	double halfWidth = ((this->m_numOfLanes * DGraphicsScene::laneWidth) - this->cellWidthBuffer * 2) * this->m_scale / 2;
	this->setRect(-halfWidth, -halfWidth, halfWidth * 2, halfWidth * 2);

	// update the four corner resizing area and the rotation definition area region
	updateResizeHandles();
	updateRotateHandles();
	this->update();
}

// --- --- --- --- --- Setting --- --- --- --- ---
void DDemandSinkCellItem::setTypeOfCell(CellType type) {
	this->typeOfCell = type;

	if (this->typeOfCell == CellType::Demand) {
		m_Color = m_DmdCellcolor;
	}
	else if (this->typeOfCell == CellType::Sink) {
		m_Color = m_SnkCellcolor;
	}
	// then update the brush color
	this->update();
}

void DDemandSinkCellItem::setCellID(int CellID){
	this->m_cellId = CellID;
}

void DDemandSinkCellItem::setMovableRotatable(bool MovableAndRotatable) {
	this->m_isMovableRotatable = MovableAndRotatable;
	if (MovableAndRotatable == true) {
		this->setFlag(QGraphicsItem::ItemIsMovable, true);
	}
	else {
		this->setFlag(QGraphicsItem::ItemIsMovable, false);
	}
}

void DDemandSinkCellItem::setUseCustomColor(bool isUse, QColor color) {
	this->m_useCustomColor = isUse;
	this->m_customColor = std::move(color);
}

void DDemandSinkCellItem::setUseText(bool isUse, QString&& str) {
	this->m_useCustomText = isUse;
	this->m_customText = std::move(str);
}

void DDemandSinkCellItem::setnumOfLanes(int numOfLanes) {
	this->m_numOfLanes = numOfLanes;
}

void DDemandSinkCellItem::setScale(double ratio) {
	// coz it would probably be uninitialized in core; in which the VisInfo_pt.m_scale = -1
	this->m_scale = abs(ratio);
}

// --- --- --- --- --- Override QGraphicsItem --- --- --- --- ---
int DDemandSinkCellItem::type() const {
	return UserType + DGraphicsScene::ZValue_GraphicItemLayer::DmdSnkCellItem;
}

void DDemandSinkCellItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	// get the newly updated QRectF shape
	QRectF original_rect = this->rect();

	//style
	painter->setPen(this->m_pen);
	this->m_brush.setColor(this->m_useCustomColor? this->m_customColor : (this->isSelected() ? this->m_selectedColor : this->m_Color));
	painter->setBrush(this->m_brush);

	// draw the filled rounded rectangle
	painter->drawRoundedRect(original_rect, 
		original_rect.width() *std::max(filledRatio, filledRatio * log (this->m_scale)),
		original_rect.width()* std::max(filledRatio, filledRatio * log(this->m_scale)) );
	// better pixel drawing off
	painter->setRenderHint(QPainter::Antialiasing, false);
	
	//do the tiny half arrow to show the direciton
	double startX = this->rect().center().x() / 2;
	this->m_brush.setColor(this->m_arrowColor);
	painter->setBrush(this->m_brush);
	QPolygonF leftPointyThing{};
	QPolygonF rightPointyThing{};
	//pre-cal some stuff
	double xBack = startX - std::max((this->arrowLength / 2), (this->arrowLength * this->m_scale * 0.4 / 2));
	double xFront = startX + std::max((this->arrowLength / 2), (this->arrowLength * this->m_scale * 0.4 / 2));
	//set the pointy thing's vertices coords
	leftPointyThing << QPointF{ xBack, -(this->rect().width() / 2) }
		<< QPointF{ xFront, -(this->rect().width() / 2) }
	<< QPointF{ xBack, -(this->rect().width() / 2) + std::max(this->arrowWidth , (this->arrowWidth * this->m_scale * 0.4)) };
	rightPointyThing << QPointF{ xBack, (this->rect().width() / 2) }
		<< QPointF{ xFront,  (this->rect().width() / 2) }
	<< QPointF{ xBack,  (this->rect().width() / 2) - std::max(this->arrowWidth , (this->arrowWidth * this->m_scale * 0.4)) };
	// draw half arrow
	painter->drawPolygon(leftPointyThing);
	painter->drawPolygon(rightPointyThing);

	//custom text
	if (this->m_useCustomText) {
		//cache the pt size and pos adjustment
		if (!this->m_textCacheIsValid) {
			//figure out the pt size
			int flags = Qt::TextDontClip;
			QRect fontBoundRect = painter->fontMetrics().boundingRect(this->rect().toRect(), flags, "00.000");
			float xFactor = this->rect().width() * 0.95 / fontBoundRect.width();
			float yFactor = this->rect().height() * 0.95 / fontBoundRect.height();
			float factor = xFactor < yFactor ? xFactor : yFactor;
			QFont f = painter->font();
			this->m_ptSize = f.pointSizeF() * factor * 0.95;
			f.setPointSizeF(this->m_ptSize);
			painter->setFont(f);

			//get the final text rect and cal pos
			QRect finalFontBoundRect = painter->fontMetrics().boundingRect(this->rect().toRect(), flags, "00.000");
			this->m_textTransform.reset();
			double dx = (this->rect().width() - finalFontBoundRect.width()) / 2;
			double dy = -(this->rect().height() - finalFontBoundRect.height()) / 2;
			this->m_textTransform.translate(dx, dy);

			this->m_textCacheIsValid = true;
		}

		//font and pos
		QFont f = painter->font();
		f.setPointSizeF(this->m_ptSize);
		painter->setFont(f);
		painter->setTransform(this->m_textTransform, true);
		//color
		//source: https://mixable.blog/black-or-white-text-on-a-colour-background/
		int r, g, b;
		(this->m_useCustomColor ? this->m_customColor : (this->isSelected() ? this->m_selectedColor : this->m_Color)).getRgb(&r, &g, &b);
		painter->setPen((r * 0.299 + g * 0.587 + b * 0.114) >= 128 ? Qt::black : Qt::white);

		painter->drawText(this->rect().bottomLeft(), this->m_customText);
		painter->setTransform(this->m_textTransform.inverted(), true);
	}

	if (this->m_isMovableRotatable) {
		// draw resize rescale control area and place at four corner region 
		painter->setBrush(this->m_brush);
		painter->drawRect(this->topLeftRect);
		painter->drawRect(this->topRightRect);
		painter->drawRect(this->bottomLeftRect);
		painter->drawRect(this->bottomRightRect);

		// draw line from center to rotate control area 
		painter->setPen(QPen(Qt::red, 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
		painter->drawLine(QLineF(this->boundingRect().center(), this->RotateRect.center()));

		// draw rotate control area on the right
		painter->setBrush(Qt::red);
		painter->drawRect(this->RotateRect);

		// draw rotate control center area
		painter->setPen(QPen(this->m_lineColor, 0, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin));
		painter->setBrush(Qt::NoBrush);
		painter->drawEllipse(this->rect().center(), this->rect().width() / 10, this->rect().width() / 10);

		// draw rotate control center point
		painter->setPen(QPen(Qt::red, this->m_numOfLanes * DGraphicsScene::laneWidth / 40));
		painter->drawPoint(0, 0);
	}
}

QRectF DDemandSinkCellItem::boundingRect() const {
	return QGraphicsRectItem::boundingRect().normalized();
}

void DDemandSinkCellItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event){
	if (m_isMovableRotatable) {
		if (m_isResizing) {
			// update the four corner resizing area and the rotation definition area region
			updateResizeHandles();
			updateRotateHandles();

			//// --- --- --- --- --- --- Resize --- --- --- --- --- ---
			int dx = int(2.0 * event->pos().x());
			int dy = int(2.0 * event->pos().y());
			prepareGeometryChange();

			QRectF rect = this->rect();
			
			//// --- --- --- --- --- --- Method1: width and height rescale freely --- --- --- --- --- ---
			///// TODO future coding; got bugs to fix for rescaling drag when item is pos-rotated
			//QPointF pos = event->scenePos() - scenePos();
			//rect.setRight(pos.x() / this->scale());  // default pos.x() / 1
			//rect.setBottom(pos.y() / this->scale());  // default pos.y() / 1


			// --- --- --- --- --- --- Method2: width and height r tied together, will be rescale at same extent --- --- --- --- --- ---
			double distance_OriginalitemPtToMovingpt = sqrt(pow((event->scenePos().x() - scenePos().x()), 2) + pow((event->scenePos().y() - scenePos().y()), 2));

			// to refrain the max min size of the square rectangle
			if (distance_OriginalitemPtToMovingpt < MIN_ITEM_SIZE) {
				distance_OriginalitemPtToMovingpt = MIN_ITEM_SIZE;
			}
			else if (distance_OriginalitemPtToMovingpt > MAX_ITEM_SIZE) {
				distance_OriginalitemPtToMovingpt = MAX_ITEM_SIZE;
			}
			
			rect.setRight(distance_OriginalitemPtToMovingpt / this->scale());  // default pos.x() / 1
			rect.setBottom(distance_OriginalitemPtToMovingpt / this->scale());  // default pos.y() / 1


			// move the rectangle back to center
			rect.moveCenter(QPointF(0, 0));
			this->setRect(rect);

			//update geometry change ratio scale
			double original_Width = (this->m_numOfLanes * DGraphicsScene::laneWidth)  - this->cellWidthBuffer*2;
			this->m_scale = this->rect().width() / original_Width;
		}
		else if (m_isRotating) {
			// update the four corner resizing area and the rotation definition area region
			updateResizeHandles();
			updateRotateHandles();

			// --- --- --- --- --- --- Rotate --- --- --- --- --- ---
			QPointF cursorPos = event->scenePos();
			double degrees = qRadiansToDegrees(-std::atan2(cursorPos.x() - this->pos().x(), cursorPos.y() - this->pos().y())) + 90;
			this->setRotation(degrees);
		}
		else {
			QGraphicsItem::mouseMoveEvent(event);
		}
		// emit Customized Signal for function to capture
		emit ShapePosChange();
	}
	
}

void DDemandSinkCellItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event){
	// switching the cursor icon
	if (m_isMovableRotatable) {
		// item coordinates 
		if (m_isResizing || (isInResizeArea(event->pos()) && isSelected())) {
			if ( event->scenePos().x() > this->mapToScene(this->rect().center()).x() && event->scenePos().y() < this->mapToScene(this->rect().center()).y() ||
				event->scenePos().x() < this->mapToScene(this->rect().center()).x() && event->scenePos().y() > this->mapToScene(this->rect().center()).y() ) {
				this->setCursor(Qt::SizeBDiagCursor);
			}
			else if ( event->scenePos().x() < this->mapToScene(this->rect().center()).x() && event->scenePos().y() < this->mapToScene(this->rect().center()).y() ||
				event->scenePos().x() > this->mapToScene(this->rect().center()).x() && event->scenePos().y() > this->mapToScene(this->rect().center()).y() ) {
				this->setCursor(Qt::SizeFDiagCursor);
			}
			
		}
		else if ((isInRotateArea(event->pos()) && isSelected())) {
			QCursor* rotateCursor = new QCursor(QPixmap("icons/rotate_cursor_icon_25px.png"));
			this->setCursor(*rotateCursor);
		}
		else {
			this->setCursor(Qt::SizeAllCursor);
		}
		QGraphicsItem::hoverMoveEvent(event);
	}
	else {
		this->setCursor(Qt::ArrowCursor);
	}
}

void DDemandSinkCellItem::mousePressEvent(QGraphicsSceneMouseEvent* event){
	if (event->button() == Qt::LeftButton && isInResizeArea(event->pos())){
		m_isResizing = true;
	}
	else if (event->button() == Qt::LeftButton && isInRotateArea(event->pos())){
		m_isRotating = true;
	}
	else{
		QGraphicsItem::mousePressEvent(event);
	}
}

void DDemandSinkCellItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event){
	if (event->button() == Qt::LeftButton && m_isResizing){
		m_isResizing = false;
	}
	else if (event->button() == Qt::LeftButton && m_isRotating){
		m_isRotating = false;
	}
	else{
		QGraphicsItem::mouseReleaseEvent(event);
	}
}


