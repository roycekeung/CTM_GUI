#pragma once

#include <unordered_set>

#include <QWidget>
#include "ui_DGenQueueLengthPanel.h"

class DGenQueueLengthFunction;
class DOkCancelButtons;

class DGenQueueLengthPanel : public QWidget {
	Q_OBJECT
private:
	// --- --- --- --- --- Hint Texts --- --- --- --- ---

	const static std::string hintText;

	// --- --- --- --- --- UI Items --- --- --- --- ---

	Ui::DGenQueueLengthPanel ui;
	DOkCancelButtons* m_okButtons;

public:
	
	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DGenQueueLengthPanel();

	~DGenQueueLengthPanel();

	void connectFunction(DGenQueueLengthFunction* function);

	// --- --- --- --- --- Settings for Functions --- --- --- --- ---

	void setRecStartEndTime(int startTime, int endTime);

	void setStartCellId(int cellId);

	void resetList(const std::unordered_set<int>& cellIds);

	// --- --- --- --- --- Getters --- --- --- --- ---

	int getStartTime();

	int getEndTime();

	double getThresholdRate();

signals:

	void removeEndCell(int cellId);

public slots:

	// --- --- --- --- --- Redirect Slots --- --- --- --- ---

	void clickedRemove();

};
