#pragma once

#include <QWidget>
#include "ui_DHintClosePanel.h"

class DHintClosePanel : public QWidget {
	Q_OBJECT

private:
	Ui::DHintClosePanel ui;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DHintClosePanel(QString&& groupBoxText = "");

	~DHintClosePanel();

	// --- --- --- --- --- Getters / Setters --- --- --- --- ---

	QPushButton* getCloseButton();

	void setHintText(QString&& text);

	void setFeedbackMessage(QString&& msg);

};
