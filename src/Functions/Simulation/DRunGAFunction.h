#pragma once

#include <map>

#include <QObject>

#include "Functions/I_Function.h"


class DGASettingsPanel;
namespace DISCO2_GA {
class GA_Module;
}
namespace DISCO2_GA_coreConvertor {
class SigDataConvertor_Core;
class Runner_CoreMultiThread;
}

class DRunGAFunction : public QObject, public I_Function {
	Q_OBJECT

private:

	DGASettingsPanel* m_panelWidget = nullptr;

	std::map<QString, int> m_sigSetStrings;		//insert order required for show default param

	std::map<QString, int> m_dmdSetStrings;

	DISCO2_GA::GA_Module* m_GAModule = nullptr;

	DISCO2_GA_coreConvertor::SigDataConvertor_Core* ref_sigConvertor = nullptr;
	DISCO2_GA_coreConvertor::Runner_CoreMultiThread* ref_runner = nullptr;

	size_t m_savedIndex = 0;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DRunGAFunction();

	~DRunGAFunction();

	void initFunctionHandler() override;

public slots:

	// --- --- --- --- --- Checking and creation Functions --- --- --- --- ---

	void checkAndRunGA();

	void saveSigSet();

	void cancel();

	void closeAndOpenRecView();

private:

	// --- --- --- --- --- Private Utils --- --- --- --- ---

	void showDefaultSettings();

	QList<QString> genSigString();

	QList<QString> genDmdString();

};
