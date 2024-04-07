#include "DNewScenFunc.h"

// std lib
#include <stdexcept>
// DISCO_GUI lib
#include "DFunctionHandler.h"
#include "Functions/File/DSaveScenFunc.h"
#include "DGraphicsScene.h"
#include "Functions/DBaseMapFunction.h"
#include "Utils/TileGenerator_GeoInfoMap_HK1980.h"
// Qt lib
#include <QMessageBox>
#include <QInputDialog>
#include <QPushButton>

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DNewScenFunc::DNewScenFunc(int simTSize) : m_simTSize(simTSize) {}

DNewScenFunc::~DNewScenFunc() {}

// --- --- --- --- --- override initFunctionHandler from I_Function --- --- --- --- ---

void DNewScenFunc::initFunctionHandler() {
	bool makeNew = true;

	//# Save any exsisting network first
	if (this->ref_scn->get()->getNetwork().getCellIds().size() || this->ref_scn->get()->getNetwork().getJctIds().size()) {
		QMessageBox* messageBox = new QMessageBox(nullptr);

		// basic UI set up
		messageBox->setWindowTitle("New Scenario MessageBox");
		messageBox->setText("New Scenario will be created.");
		messageBox->setInformativeText("Do you want to save your changes?");
		messageBox->resize(350, 200);
		messageBox->setStandardButtons(
			QMessageBox::StandardButton::Save | QMessageBox::StandardButton::Cancel | QMessageBox::StandardButton::Discard);

		// turn off the built-in help question mark button
		messageBox->setWindowFlags(messageBox->windowFlags() & ~Qt::WindowContextHelpButtonHint);

		//run the message box and will return the QMessageBox::Role of what button u press on
		switch (messageBox->exec()) {
		case QMessageBox::StandardButton::Save:
			this->ref_functionHandler->addFunction(new DSaveScenFunc{});	//should be blocking?
			break;

		case QMessageBox::StandardButton::Cancel:
			makeNew = false;
			break;

		default:
		case QMessageBox::StandardButton::Discard:
			break;
		}

		delete messageBox;
	}

	//# create the new scn
	if (makeNew) {
		int simRes = 1000;
		//check for cstr param
		if (this->m_simTSize > 0 && 1000 % this->m_simTSize == 0) {
			simRes = this->m_simTSize;
		}
		else {
#ifndef LOCKED_GUI_DEPLOY
			//ask for simRes
			bool isValid = false;
			while (!isValid) {
				//TODO change to full impl to remove the cancel button
				double input = QInputDialog::getDouble(nullptr, 
					"Set simulation resolution", "Time Step Size: (sec) (1 should be divisible by the inputted number)", 
					1, 0, 1, 3);
				//imperfect check of input value with floating pt bs
				if (1000 % (int)std::round(input*1000) == 0) {
					simRes = (int)(input * 1000);
					isValid = true;
				}
			}
#endif	//  ifndef LOCKED_GUI_DEPLOY
		}

		try {
			//make the scenario
			this->ref_scn->swap(DISCO2_API::Scenario::createNewScenario());
			this->ref_scn->get()->getDmdEditor().dmdSet_create();
			this->ref_scn->get()->getSigEditor().sigSet_create();

			//FIXME hard coded all HK1980
			DISCO2_API::VisInfo visInfo = this->ref_scn->get()->getNetwork().getVisInfo();
			visInfo.addSecret("CoordSys", "HK1980");
			this->ref_scn->get()->getNetEditor().net_setVisInfo(std::move(visInfo));
			//do the baseMap
			this->ref_coordTransform->setCoordSystem(CoordTransform::CoordSystem::HK1980);
			this->ref_baseMapFunction->setTileGenerator(new TileGenerator_GeoInfoMap_HK1980{ this->ref_coordTransform });

			// rebuild the Graphicscene
			this->ref_graphicsScene->reloadNet();
			this->ref_graphicsScene->update();

			//potentially throw
			this->ref_scn->get()->getNetEditor().net_setTimeStepSize(simRes);
		}
		catch (std::exception& e) {
			// print out corresponding error
			QString Error_name = QString::fromStdString(e.what());
			QMessageBox::critical(nullptr, "Error", Error_name);
		}
	}

	//# destroy function
	this->closeFunction();
}
