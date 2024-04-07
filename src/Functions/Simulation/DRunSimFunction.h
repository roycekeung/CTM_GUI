#pragma once

#include <map>

#include <QObject>

#include "Functions/I_Function.h"

class DSimSettingsPanel;

class DRunSimFunction : public QObject, public I_Function {
	Q_OBJECT

private:

	DSimSettingsPanel* m_panelWidget = nullptr;

	std::map<QString, int> m_sigSetStrings;		//insert order required for show default param

	std::map<QString, int> m_dmdSetStrings;

	size_t m_savedIndex = 0;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DRunSimFunction();

	~DRunSimFunction();

	void initFunctionHandler() override;

public slots:

	// --- --- --- --- --- Checking and creation Functions --- --- --- --- ---

	void checkAndRunSim();

	void cancel();

	void closeAndOpenRecView();

private:

	// --- --- --- --- --- Private Utils --- --- --- --- ---

	void showDefaultSettings();

	QList<QString> genSigString();

	QList<QString> genDmdString();

};
