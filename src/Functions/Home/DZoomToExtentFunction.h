#pragma once

// DISCO2_GUI lib
#include "../Functions/I_Function.h"

class DZoomToExtentFunction : public I_Function {

public:

	DZoomToExtentFunction();
	~DZoomToExtentFunction();

	// --- --- --- --- --- override initFunctionHandler from I_Function --- --- --- --- ---
	void initFunctionHandler() override;

};
