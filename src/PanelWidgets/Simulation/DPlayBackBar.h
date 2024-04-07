#pragma once

#include <QWidget>
#include "ui_DPlayBackBar.h"
#include <QtWidgets\qpushbutton.h>

class DPlayBackFunction;

class DPlayBackBar : public QWidget {
	Q_OBJECT
private:

	int64_t m_startTime = -1;
	int64_t m_endTime = -1;
	int m_timeStepSize = -1;

	// --- --- --- --- --- UI Items --- --- --- --- ---

	Ui::DPlayBackBar ui;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DPlayBackBar();

	~DPlayBackBar();

	// --- --- --- --- --- Setters --- --- --- --- ---

	void connectFunction(DPlayBackFunction* function);

	void setStartEnd(int64_t start, int64_t end, int tSize);

	void setCurrentTime(int64_t time);

	void setSpeed(int value);

};
