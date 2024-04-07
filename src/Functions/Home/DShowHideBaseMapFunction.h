#pragma once

// DISCO2_GUI lib
#include "../Functions/I_Function.h"

class QPushButton;

class DShowHideBaseMapFunction : public I_Function {
private:

	QPushButton* ref_showHideButton;

public:

	DShowHideBaseMapFunction(QPushButton* showHideButton);
	~DShowHideBaseMapFunction();

	// --- --- --- --- --- override initFunctionHandler from I_Function --- --- --- --- ---

	void initFunctionHandler() override;

};

