#pragma once

// DISCO_GUI lib
#include "../Functions/I_Function.h"

#include <QString>

class DLoadScenFunc : public I_Function {
private:
	QString m_fileName;

public:
	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DLoadScenFunc(QString fileName = {});

	~DLoadScenFunc() = default;

	// --- --- --- --- --- override initFunctionHandler from I_Function --- --- --- --- ---

	void initFunctionHandler() override;

	// DLoadScenFunc QWidget is closed anyway coz didnt even shows up thru UI
};
