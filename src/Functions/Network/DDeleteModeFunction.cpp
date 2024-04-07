#include "DDeleteModeFunction.h"

#include <QPushButton>

#include "Functions/DBaseNaviFunction.h"

DDeleteModeFunction::DDeleteModeFunction(QPushButton* deleteButton) : QObject(nullptr), I_Function(), m_deleteButton(deleteButton) {}

DDeleteModeFunction::~DDeleteModeFunction() {}


void DDeleteModeFunction::initFunctionHandler() {
	//check base navi instead of the button as that is true record
	bool isOrgDeleteMode = this->ref_baseNaviFunction->getIsDeleteMode();

	//change the button
	this->m_deleteButton->setChecked(!isOrgDeleteMode);
	//change base navi
	this->ref_baseNaviFunction->setDeleteMode(!isOrgDeleteMode);

	//done and delete
	this->closeFunction();
}