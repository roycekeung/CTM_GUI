#include "DSigDemandFileDialogue.h"

#include "Functions/File/DSigDemandFileFunction.h"

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DSigDemandFileDialogue::DSigDemandFileDialogue() : QDialog() {
	ui.setupUi(this);

	this->setWindowFlags(Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
	this->setAttribute(Qt::WA_DeleteOnClose);
}

DSigDemandFileDialogue::~DSigDemandFileDialogue() {}

Ui::DSigDemandFileDialogue DSigDemandFileDialogue::getUI() {
	return this->ui;
}
