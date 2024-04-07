#pragma once

#include <QWidget>
#include "ui_DGenDelayPlotPanel.h"

class DGenDelayPlotFunction;

class DGenDelayPlotPanel : public QWidget {
	Q_OBJECT
private:
	// --- --- --- --- --- Hint Texts --- --- --- --- ---

	const static std::string hintText;

	// --- --- --- --- --- UI Items --- --- --- --- ---

	Ui::DGenDelayPlotPanel ui;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DGenDelayPlotPanel();

	~DGenDelayPlotPanel();

	void connectFunction(DGenDelayPlotFunction* function);

	// --- --- --- --- --- Settings for Functions --- --- --- --- ---

	void addRow(int cellId);

	void setRecStartEndTime(int startTime, int endTime);

	int getPlotType();

	int getStartTime();

	int getEndTime();

	void removeLastRow();

	void removeAllRows();

};
