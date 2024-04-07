#pragma once

#include <QObject>

#include <I_Function.h>

class DManageRunPanel;

class DManageRunFunction : public QObject, public I_Function {
	Q_OBJECT

private:
	DManageRunPanel* m_panelWidet = nullptr;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DManageRunFunction();

	~DManageRunFunction();

	void initFunctionHandler() override;

public slots:

	// --- --- --- --- --- Slots for Panel --- --- --- --- ---

	void openPlayback();

	void deleteRun();

	void done();

private:

	// --- --- --- --- --- Private Util --- --- --- --- ---

	void reloadTable();

};
