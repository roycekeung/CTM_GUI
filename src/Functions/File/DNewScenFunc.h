#pragma once

#include "Functions/I_Function.h"

class DNewScenFunc : public I_Function {
private:

	int m_simTSize;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	/**
	 Function will handle unsaved exsisting network
	 Creates new scn
	 Optional parameter simTSize: 
		0 | invalid values -> will prompt user to input simTSize
	*/
	DNewScenFunc(int simTSize = 0);

	~DNewScenFunc();

	// --- --- --- --- --- override initFunctionHandler from I_Function --- --- --- --- ---
	void initFunctionHandler() override;

};

