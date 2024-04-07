#include "DHintClosePanel.h"

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DHintClosePanel::DHintClosePanel(QString&& groupBoxText) : QWidget(nullptr) {
	ui.setupUi(this);
	this->ui.groupBox->setTitle(groupBoxText);
}

DHintClosePanel::~DHintClosePanel() {
}

// --- --- --- --- --- Getters / Setters --- --- --- --- ---

QPushButton* DHintClosePanel::getCloseButton() {
	return this->ui.close_pushButton;
}

void DHintClosePanel::setHintText(QString&& text) {
	this->ui.hint_label->setText(text);
}

void DHintClosePanel::setFeedbackMessage(QString&& msg) {
	this->ui.error_label->setText(msg);
}
