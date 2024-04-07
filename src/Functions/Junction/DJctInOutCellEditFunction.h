#pragma once

#include <QObject>

#include "../I_Function.h"

class DJctInOutCellPanel;

class DJctInOutCellEditFunction : public QObject, public I_Function {
	Q_OBJECT

private:

	int m_jctId;
	int m_cellId;

	DJctInOutCellPanel* m_panelWidet = nullptr;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DJctInOutCellEditFunction(int jctId, int cellId);

	~DJctInOutCellEditFunction();

	void initFunctionHandler() override;

public slots:

	// --- --- --- --- --- Checking and creation Functions --- --- --- --- ---

	void checkAndEditCell();

	void cancel();

public:

	// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---

	bool keyPressEvent(QKeyEvent* keyEvent) override;

};
