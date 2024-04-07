#pragma once

// std lib
#include<unordered_map>  

// Qt lib
#include <QObject>
#include <QCloseEvent>
#include <QDialog>
#include <QList>
#include <QString>


// DISCO_GUI lib
#include "../Functions/I_Function.h"

class DDefaultParamSettingDialogue;

class DDefaultParamSettingFunction : public QObject, public I_Function {
	Q_OBJECT

private:
	// --- --- --- --- --- UI pannel --- --- --- --- ---
	DDefaultParamSettingDialogue* m_DialogueWigdet = nullptr;

	std::unordered_map<QString, int> m_sigSetStrings;
	std::unordered_map<QString, int> m_dmdSetStrings;

	// --- --- --- --- --- Private Utils --- --- --- --- ---
	QList<QString> genSigString();
	QList<QString> genDmdString();

public:
	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---
	DDefaultParamSettingFunction();
	~DDefaultParamSettingFunction();

	// --- --- --- --- --- override initFunctionHandler from I_Function --- --- --- --- ---
	void initFunctionHandler() override;

public slots:
	// --- --- --- --- --- Checking and creation Functions --- --- --- --- ---
	void checkAndSubmitChanges();
	// --- --- --- --- --- Slots for Panel --- --- --- --- ---
	void dialogFinished();

public:
	// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---
	bool keyPressEvent(QKeyEvent* keyEvent) override;


};
