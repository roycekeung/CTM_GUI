#include "DAddRemoveButtons.h"

DAddRemoveButtons::DAddRemoveButtons(QWidget* parent) : QWidget(parent) {
	ui.setupUi(this);

	this->AddButton = ui.Add_pushButton;
	this->RemoveButton = ui.Remove_pushButton;
}

DAddRemoveButtons::~DAddRemoveButtons() {
}