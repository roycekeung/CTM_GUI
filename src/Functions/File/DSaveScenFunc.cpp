#include "DSaveScenFunc.h"

// std lib
#include <stdexcept>
// Qt lib
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>

DSaveScenFunc::DSaveScenFunc() {
}

void DSaveScenFunc::initFunctionHandler() {
	QString fileName = QFileDialog::getSaveFileName(nullptr, QObject::tr("Save Scenario"), QDir::currentPath(), QObject::tr("XML files (*.xml)"));

	if (!fileName.isEmpty()) {
		// save file in, got the file name

		try {
			this->ref_scn->get()->saveScenario(fileName.toStdString());
		}
		catch (std::exception& e) {
			// print out corresponding error
			QMessageBox ErrormessageBox;
			QString Error_name = QString::fromStdString(e.what());
			ErrormessageBox.critical(nullptr, "Error", Error_name);
			ErrormessageBox.setFixedSize(500, 200);
		}
	}
	// else, didnt even save any file in, u cancel the action

	// destroy function
	this->closeFunction();
}
