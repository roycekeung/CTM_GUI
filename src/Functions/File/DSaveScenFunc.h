#pragma once

// DISCO_GUI lib
#include "../Functions/I_Function.h"

class DSaveScenFunc : public I_Function {
public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---
	DSaveScenFunc();
	~DSaveScenFunc() = default;

	// --- --- --- --- --- override initFunctionHandler from I_Function --- --- --- --- ---
	void initFunctionHandler() override;

};

