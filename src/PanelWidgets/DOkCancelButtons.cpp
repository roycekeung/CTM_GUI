#include "DOkCancelButtons.h"

DOkCancelButtons::DOkCancelButtons(QWidget *parent) : QWidget(parent) {
	ui.setupUi(this);

	this->ref_feedbackLabel = ui.Feedback_Label;
	this->ref_confirmButton = ui.Confirm_Button;
	this->ref_cancelButton = ui.Cancel_Button;
}

DOkCancelButtons::~DOkCancelButtons() {
}
