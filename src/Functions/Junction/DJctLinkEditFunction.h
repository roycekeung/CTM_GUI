#pragma once

#include <QObject>

#include "../I_Function.h"

class DArcPanel;

class DJctLinkEditFunction : public QObject, public I_Function {
	Q_OBJECT

private:

	int m_jctId;
	int m_jctLinkId;

	DArcPanel* m_panelWidet = nullptr;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DJctLinkEditFunction(int jctId, int jctLinkId);

	~DJctLinkEditFunction();

	void initFunctionHandler() override;

public slots:

	// --- --- --- --- --- Checking and creation Functions --- --- --- --- ---

	void checkAndEditJctLink();

	void cancel();

	void recalLength();

};
