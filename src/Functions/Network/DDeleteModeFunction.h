#pragma once

#include <QObject>
class QPushButton;

#include "Functions/I_Function.h"

class DDeleteModeFunction : public QObject, public I_Function {
	Q_OBJECT

private:

	QPushButton* m_deleteButton;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DDeleteModeFunction(QPushButton* deleteButton);

	~DDeleteModeFunction();

	void initFunctionHandler() override;

};