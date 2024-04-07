#pragma once

#include <QObject>

#include "../I_Function.h"

class DCellPanel;

class QKeyEvent;

class DCellEditFunction : public QObject, public I_Function {
	Q_OBJECT

private:

	int m_cellId;

	DCellPanel* m_panelWidet = nullptr;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DCellEditFunction(int cellId);

	~DCellEditFunction();

	void initFunctionHandler() override;

public slots:

	// --- --- --- --- --- Checking and creation Functions --- --- --- --- ---

	void checkAndEditCell();

	void cancel();

public:

	// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---

	bool keyPressEvent(QKeyEvent* keyEvent) override;

};
