#pragma once

#include <QWidget>
#include "ui_DOkCancelButtons.h"

class DOkCancelButtons : public QWidget {
	Q_OBJECT

public:

	QLabel* ref_feedbackLabel;
	QPushButton* ref_confirmButton;
	QPushButton* ref_cancelButton;

	DOkCancelButtons(QWidget *parent = Q_NULLPTR);
	~DOkCancelButtons();

private:
	Ui::DOkCancelButtons ui;
};
