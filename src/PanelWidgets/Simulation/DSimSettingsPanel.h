#pragma once

#include <QWidget>
#include "ui_DSimSettingsPanel.h"

//DISCO GUI stuff
class DOkCancelButtons;
class DRunSimFunction;

class DSimSettingsPanel : public QWidget {
	Q_OBJECT
private:

	// --- --- --- --- --- Hint Texts --- --- --- --- ---

	const static std::string hintText;

	// --- --- --- --- --- UI Items --- --- --- --- ---

	Ui::DSimSettingsPanel ui;

	QLabel* hint_Label = nullptr;
	DOkCancelButtons* ok_Buttons = nullptr;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DSimSettingsPanel(const QList<QString>& sigSetEntries, const QList<QString>& dmdSetEntries);

	~DSimSettingsPanel();

	// --- --- --- --- --- Settings for Function --- --- --- --- ---

	void setupButtons(DRunSimFunction* function);

	void failMessage(QString&& msg);

	void showResults(bool isShow, double tolDelay = 0, double tolDemand = 0, double avgDelay = 0);

	// --- --- --- --- --- Getters for relaying input form values --- --- --- --- ---

	QString getSigSetInput();
	QString getDmdSetInput();
	int getTimeInput();
	QString getNameInput();
	QString getDetailsInput();

	// --- --- --- --- --- Setters for default settings --- --- --- --- ---

	void setSigSetInput(int index);
	void setDmdSetInput(int index);
	void setTimeInput(int time);

};
