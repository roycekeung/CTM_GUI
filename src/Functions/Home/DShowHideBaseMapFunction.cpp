#include "DShowHideBaseMapFunction.h"

#include "Functions/DBaseMapFunction.h"

#include <QPushButton>

DShowHideBaseMapFunction::DShowHideBaseMapFunction(QPushButton* showHideButton) : I_Function(), ref_showHideButton(showHideButton) {}

DShowHideBaseMapFunction::~DShowHideBaseMapFunction() {}

// --- --- --- --- --- override initFunctionHandler from I_Function --- --- --- --- ---

void DShowHideBaseMapFunction::initFunctionHandler() {
	bool orgIsShow = this->ref_baseMapFunction->getIsShowBaseMap();

	if (orgIsShow)
		this->ref_showHideButton->setText("BaseMap Off");
	else
		this->ref_showHideButton->setText("BaseMap On");

	this->ref_showHideButton->setChecked(!orgIsShow);
	this->ref_baseMapFunction->setIsShowBaseMap(!orgIsShow);

	this->closeFunction();
}
