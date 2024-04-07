#include "DNetLockFunction.h"

#include <QPushButton>

#include "Functions/DBaseNaviFunction.h"

DNetLockFunction::DNetLockFunction(QPushButton* lockButton) : QObject(nullptr), I_Function(), m_lockButton(lockButton) {
}

DNetLockFunction::~DNetLockFunction() {
}

void DNetLockFunction::initFunctionHandler() {
	//check base navi instead of the button as that is true record
	if (this->ref_baseNaviFunction->getIsNetEditMode()) {
		//change the button
		this->m_lockButton->setChecked(true);
		this->m_lockButton->setText("Net Locked");
		//change base navi
		this->ref_baseNaviFunction->setNetEditMode(false);
	}
	else {
		//change the button
		this->m_lockButton->setChecked(false);
		this->m_lockButton->setText("Net Unlocked");
		//change base navi
		this->ref_baseNaviFunction->setNetEditMode(true);
	}

	//done and delete
	this->closeFunction();
}
