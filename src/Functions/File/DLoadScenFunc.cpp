#include "DLoadScenFunc.h"

// std lib
#include <stdexcept>
// DISCO GUI
#include "DGraphicsScene.h"
#include "DGraphicsView.h"
#include "Functions/DBaseMapFunction.h"
#include "Utils/TileGenerator_GeoInfoMap_HK1980.h"
#include "DFunctionHandler.h"
#include "Functions/Home/DZoomToExtentFunction.h"
// Qt lib
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>

class DGraphicsScene;

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DLoadScenFunc::DLoadScenFunc(QString fileName) : m_fileName(fileName) {}

// --- --- --- --- --- override initFunctionHandler from I_Function --- --- --- --- ---

void DLoadScenFunc::initFunctionHandler() {
	if(this->m_fileName.isEmpty())
		this->m_fileName = QFileDialog::getOpenFileName( nullptr, QObject::tr("Load Scenario"), QDir::currentPath(), QObject::tr("XML files (*.xml)"));

	if (!this->m_fileName.isEmpty()) {
		// save file in, got the file name
		try {
			this->ref_scn->swap(this->ref_scn->get()->loadScenario(this->m_fileName.toStdString()));
			//setup the coord system and base map
			std::string coordSys = this->ref_scn->get()->getNetwork().getVisInfo().findSecret("CoordSys");
			if (!coordSys.compare("HK1980")) { //test net doesn't have it???
				this->ref_coordTransform->setCoordSystem(CoordTransform::CoordSystem::HK1980);
				this->ref_baseMapFunction->setTileGenerator(new TileGenerator_GeoInfoMap_HK1980{ this->ref_coordTransform });
			}
			else {
				this->ref_coordTransform->setCoordSystem(CoordTransform::CoordSystem::WGS84);
			}

			// rebuild the Graphicscene
			this->ref_graphicsScene->reloadNet();
			this->ref_graphicsScene->update();
		}
		catch (std::exception& e) {
			// print out corresponding error
			QMessageBox ErrormessageBox;
			QString Error_name = "DLoadScenFunc: " + QString::fromStdString(e.what());
			ErrormessageBox.critical(nullptr, "Error", Error_name);
			ErrormessageBox.setFixedSize(500, 200);
		}
	}
	
	//reset the view to extents
	this->ref_functionHandler->addFunction(new DZoomToExtentFunction{});
	// destroy function
	this->closeFunction();
}

