#pragma once

// Qt lib
#include <QObject>

// DISCO2_GUI lib
#include "../Functions/I_Function.h"

// pre def
class DShowHidePanel;

class DShowHideFunction : public QObject, public I_Function{
	Q_OBJECT
private:
	DShowHidePanel* m_ShowHidePanelWidet = nullptr;

	// --- --- --- --- --- Check initial State --- --- --- --- ---
	void SetInitialCheckState();

public:
	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---
	DShowHideFunction();
	~DShowHideFunction();


	// --- --- --- --- --- override initFunctionHandler from I_Function --- --- --- --- ---
	void initFunctionHandler() override;

	// --- --- --- --- --- Close Functions --- --- --- --- ---
	void cancel();

	// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---
	bool keyPressEvent(QKeyEvent* keyEvent) override;
};
