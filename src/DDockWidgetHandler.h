#pragma once

#include <unordered_map>

//Qt stuff
#include <QObject>
#include <QString>
class QMainWindow;
class QDockWidget;

class DDockWidget;

/**
 Provide easy access to creating dockWidgets

 Right dockWidget always only allow 1 widget at a time, therefore the opening and closing of that 
 widget is managed by this class.
 On closing of the right dockWidget, the stored widget inside will be deleted by this class

 Alternatively, this provides a convenience function for creating a bottom (&floatable) dockWidget 
 for other uses, this class can sort of manage the existence / life time of these dockwidgets

*/
class DDockWidgetHandler : public QObject {
	Q_OBJECT

private:
	
	// --- --- --- --- --- Ref Pointers --- --- --- --- ---

	QMainWindow* ref_mainWindow;

	DDockWidget* m_rightDockWidget;		//init as pointer as 

	std::unordered_map<DDockWidget*, QWidget*> m_floatDockWidgets;

public:
	
	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DDockWidgetHandler(QMainWindow* mainWindow);

	~DDockWidgetHandler();

	// --- --- --- --- --- Access Functions --- --- --- --- ---

	bool hasRightDockWidget();

	void setRightDockWidget(QWidget* widget, QString name = {}, bool closeable = false);

	void removeAndDeleteRightDockWidget();

	void addFloatDockWidget(QWidget* widget, QString name = {}, bool closeable = true);

	void removeAndDeleteFloatDockWidget(QWidget* widget);

signals:

	/**
	 Signal such that there's a way for functions to know a right dock widget is closed
	*/
	void rightDockWidgetClosed();

	/**
	 Signal such that there's a way for functions to know a float dock widget is closed
	 the QWidget is a deleted ptr, and is for identification only
	*/
	void floatDockWidgetClosed(QWidget* widgetPtr);

public slots:

	// --- --- --- --- --- Close Widget --- --- --- --- ---

	void closeRightDockWidget();

private slots:

	// --- --- --- --- --- Internal Widget Close Event --- --- --- --- ---

	void floatWidgetClosed(DDockWidget* ptr);

};
