#pragma once

#include <QWidget>
#include "ui_DGASettingsPanel.h"

//DISCO GUI stuff
class DOkCancelButtons;
class DRunGAFunction;

class DGASettingsPanel : public QWidget {
	Q_OBJECT

private:

	// --- --- --- --- --- Hint Texts --- --- --- --- ---

	const static std::string hintText;

	// --- --- --- --- --- UI Items --- --- --- --- ---

	Ui::DGASettingsPanel ui;

	DOkCancelButtons* ok_Buttons = nullptr;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DGASettingsPanel(const QList<QString>& sigSetEntries, const QList<QString>& dmdSetEntries);

	~DGASettingsPanel();

	// --- --- --- --- --- Settings for Function --- --- --- --- ---

	void setupButtons(DRunGAFunction* function);

	void failMessage(QString&& msg);

	void showResults(bool isShow, double tolDelay = 0, double tolDemand = 0, double avgDelay = 0);

	void sigFailMessage(QString&& msg);

	void sigSaved(int savedSigSetId);

	// --- --- --- --- --- Getters for relaying input form values --- --- --- --- ---

	QString getSigSetInput();
	QString getDmdSetInput();
	int getTimeInput();
	QString getNameInput();
	QString getDetailsInput();

	int getGAType();
	int getNumOfThreads();
	int getNumOfGen();
	int getNumOfSubGen();
	int getPopSize();
	double getEliteRate();
	double getCrossoverRate();
	double getMutationRate();
	double getPowerFactor();
	bool getOptCyc();
	bool getOptOffset();
	bool getOptGreen();
	bool getMatchCyc();

	QString getSigNameInput();
	QString getSigDetailsInput();

	// --- --- --- --- --- Setters for Default Param --- --- --- --- ---

	void setSigSetInput(int index);
	void setDmdSetInput(int index);
	void setTimeInput(int time);

	void setGAType(int index);
	void setNumOfThreads(int numOfThreads);
	void setNumOfGen(int numOfGen);
	void setNumOfSubGen(int numOfSubGen);
	void setPopSize(int popSize);
	void setEliteRate(double rate);
	void setCrossoverRate(double rate);
	void setMutationRate(double rate);
	void setPowerFactor(double powerFactor);
	void setOptCyc(bool isOptCycle);
	void setOptOffset(bool isOptOffset);
	void setOptGreen(bool isOptGreen);
	void setMatchCyc(bool isMatchCyc);

};
