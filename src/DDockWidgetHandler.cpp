#include "DDockWidgetHandler.h"

#include <QDockWidget>
#include <QMainWindow>

#include "PanelWidgets/Network/DArcPanel.h"
#include "PanelWidgets/Network/DCellPanel.h"

/**
* Quick thin wrapper class to have a close event signal
*/
class DDockWidget : public QDockWidget {
	Q_OBJECT
public:
	DDockWidget(QWidget* parent) : QDockWidget(parent) {}

signals:
	void isClosing(DDockWidget* thisPtr);

protected:
	virtual void closeEvent(QCloseEvent* event) override {
		emit this->isClosing(this);
	}

};

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DDockWidgetHandler::DDockWidgetHandler(QMainWindow* mainWindow) :
	QObject(mainWindow), ref_mainWindow(mainWindow), m_rightDockWidget(new DDockWidget{nullptr}){

	//#	init dock widget socks
	//right widget
	this->m_rightDockWidget->setFeatures(QDockWidget::DockWidgetFeature::NoDockWidgetFeatures);
	this->m_rightDockWidget->setAcceptDrops(false);
	this->m_rightDockWidget->setAllowedAreas(Qt::DockWidgetArea::RightDockWidgetArea);

	//effectively disable AllowTabbedDocks, 
	//such that they wont stack on top of each other and become unaccessible without a tab thing JLo
	this->ref_mainWindow->setDockOptions(QMainWindow::AnimatedDocks);	

	//connections
	QObject::connect(this->m_rightDockWidget, &DDockWidget::isClosing, this, &DDockWidgetHandler::closeRightDockWidget);
}

DDockWidgetHandler::~DDockWidgetHandler() {
	this->m_rightDockWidget->close();
}

// --- --- --- --- --- Access Functions --- --- --- --- ---

bool DDockWidgetHandler::hasRightDockWidget() {
	return this->m_rightDockWidget->widget();
}

void DDockWidgetHandler::setRightDockWidget(QWidget* widget, QString name, bool closeable) {
	//close existing widget
	this->closeRightDockWidget();
	//check if receiving a widget
	if (widget) {
		//add in the dock widget if not opened
		if (!this->m_rightDockWidget->isVisible())
			this->ref_mainWindow->addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, this->m_rightDockWidget);
		this->m_rightDockWidget->setWindowTitle(name);
		//dockwidget closeable setting
		this->m_rightDockWidget->setFeatures(closeable ? QDockWidget::DockWidgetFeature::DockWidgetClosable : QDockWidget::DockWidgetFeature::NoDockWidgetFeatures);
		//putin and show the requested widget
		this->m_rightDockWidget->setWidget(widget);
		this->m_rightDockWidget->show();
	}
	else {
		//close the dock widget
		this->ref_mainWindow->removeDockWidget(this->m_rightDockWidget);
	}
}

void DDockWidgetHandler::removeAndDeleteRightDockWidget() {
	this->setRightDockWidget(nullptr);
}

void DDockWidgetHandler::addFloatDockWidget(QWidget* widget, QString name, bool closeable) {
	//check if receiving a widget
	if (widget) {
		DDockWidget* out = new DDockWidget{ this->ref_mainWindow };
		//bottom widget
		out->setFeatures(QDockWidget::DockWidgetFeature::DockWidgetClosable | QDockWidget::DockWidgetFeature::DockWidgetMovable | QDockWidget::DockWidgetFeature::DockWidgetFloatable);
		out->setAttribute(Qt::WA_DeleteOnClose);
		out->setAllowedAreas(Qt::DockWidgetArea::BottomDockWidgetArea);
		//putin and show the requested widget
		out->setWidget(widget);
		//show the thing
		this->ref_mainWindow->addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, out);
		out->show();
		//record
		this->m_floatDockWidgets.emplace(out, widget);
		//connections
		QObject::connect(out, &DDockWidget::isClosing, this, &DDockWidgetHandler::floatWidgetClosed);
	}
}

void DDockWidgetHandler::removeAndDeleteFloatDockWidget(QWidget* widget) {
	for(auto& entry : this->m_floatDockWidgets)
		if (entry.second == widget) {
			entry.first->close();
			this->m_floatDockWidgets.erase(entry.first);
			emit this->floatDockWidgetClosed(widget);
			break;
		}
}

// --- --- --- --- --- Close Widget --- --- --- --- ---

void DDockWidgetHandler::closeRightDockWidget() {
	//close existing widget
	if (this->m_rightDockWidget->widget()) {
		this->m_rightDockWidget->hide();
		delete this->m_rightDockWidget->widget();
		this->m_rightDockWidget->setWidget(nullptr);
		emit this->rightDockWidgetClosed();
	}
}

// --- --- --- --- --- Internal Widget Close Event --- --- --- --- ---

void DDockWidgetHandler::floatWidgetClosed(DDockWidget* ptr) {
	if (this->m_floatDockWidgets.count(ptr)) {
		emit this->floatDockWidgetClosed(this->m_floatDockWidgets.at(ptr));
		this->m_floatDockWidgets.erase(ptr);
	}
}

// --- --- --- --- --- Such that the cpp class will work --- --- --- --- ---
#include "DDockWidgetHandler.moc"