#pragma once

#include <QObject>

#include "Functions/I_Function.h"

class QPushButton;

/**
 * Function responsible to toggle the net locking function
 * 
 * Constructor is a bit unsafe, but quick and easy impl
 * Changes the lock button toggle look and text
 */
class DNetLockFunction : public QObject, public I_Function {
	Q_OBJECT

private:

	QPushButton* m_lockButton;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DNetLockFunction(QPushButton* lockButton);

	~DNetLockFunction();

	void initFunctionHandler() override;

};
