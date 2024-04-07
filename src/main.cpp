#include <QtWidgets/QApplication>
#include <QTimer>

#include "DMainWindow.h"


int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

    DMainWindow w;
    w.show();

    //trigger once event loop starts
    QTimer::singleShot(1000, &w, &DMainWindow::newOnStartUp);

    return a.exec();
}