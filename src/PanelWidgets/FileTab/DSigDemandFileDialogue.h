#pragma once


#include <QDialog>

#include "ui_DSigDemandFileDialogue.h"

class DSigDemandFileFunction;

class DSigDemandFileDialogue : public QDialog {
	Q_OBJECT

private:

	Ui::DSigDemandFileDialogue ui;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DSigDemandFileDialogue();

	~DSigDemandFileDialogue();

	Ui::DSigDemandFileDialogue getUI();

};
