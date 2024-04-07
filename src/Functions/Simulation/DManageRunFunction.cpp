#include "DManageRunFunction.h"

#include <QDialog>

#include "PanelWidgets/Simulation/DManageRunPanel.h"
#include "DRunManager.h"
#include "DFunctionHandler.h"
#include "Functions/Simulation/DPlayBackFunction.h"

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DManageRunFunction::DManageRunFunction() : QObject(), I_Function(){
}

DManageRunFunction::~DManageRunFunction() {
}

void DManageRunFunction::initFunctionHandler() {
	this->m_panelWidet = new DManageRunPanel{};
	this->m_panelWidet->setupButtons(this);
	QObject::connect(this->m_panelWidet, &QDialog::finished, this, &DManageRunFunction::done);
	this->reloadTable();

	this->m_panelWidet->open();
}

// --- --- --- --- --- Slots for Panel --- --- --- --- ---

void DManageRunFunction::openPlayback() {
	//trigger playback function
	this->ref_functionHandler->addFunction(new DPlayBackFunction{ (size_t)this->m_panelWidet->getCurrentIndex() });

	//close and delete the panel
	this->m_panelWidet->close();
}

void DManageRunFunction::deleteRun() {
	if(this->m_panelWidet->getCurrentIndex() != -1)
		this->ref_runManager->removeAndDeleteRec(this->m_panelWidet->getCurrentIndex());
	this->reloadTable();
}

void DManageRunFunction::done() {
	this->closeFunction();
}

// --- --- --- --- --- Private Util --- --- --- --- ---

void DManageRunFunction::reloadTable() {
	this->m_panelWidet->clearTable();

	for (int i = 0; i < this->ref_runManager->size(); ++i) {
		//get all the things
		auto visInfo = this->ref_runManager->getVisinfo(i);
		auto rec = this->ref_runManager->getRec(i);

		//dump in all the things
		this->m_panelWidet->addRow(
			visInfo->getName().c_str(),
			visInfo->getDetails().c_str(),
			visInfo->findSecret("SigSet").c_str(),
			visInfo->findSecret("DmdSet").c_str(),
			visInfo->findSecret("Time").c_str(),
			rec->getRealisedDemand(),
			rec->getTotalDelay(),
			rec->getAvgDelay()
		);
	}
}
