#pragma once

// std lib
#include<unordered_map>  
// Qt lib
#include <QCloseEvent>
#include <QDialog>

#include <qtcharts/qchartview>
#include <qtcharts/qchart>
#include <qtcharts/qbarset.h>
#include <qtcharts/qhorizontalstackedbarseries.h>
#include <qtcharts/qbarcategoryaxis.h>
#include <QtCharts/qcategoryaxis.h>

// DISCO_GUI lib
#include "../Functions/I_Function.h"

class DSignalTimingPlanDialogue;


QT_CHARTS_USE_NAMESPACE   // == using namespace QtCharts;

class DSignalTimingPlanCreateFunc : public QObject, public I_Function {
	
private:

	// --- --- --- --- --- UI pannel --- --- --- --- ---
	DSignalTimingPlanDialogue* m_DialogueWigdet = nullptr;

	// Chart related ptr
	QChart* m_barChart = nullptr;
	QChartView* m_barChartView;

	QBarSet* redtimeBar;
	QBarSet* greentimeBar;
	QBarSet* redtime2Bar;
	QBarSet* greentime2Bar;
	QBarSet* redtime3Bar;
	QBarSet* greentime3Bar;

	// series for horizontalwise stacking bars
	QHorizontalStackedBarSeries* barseries;
	// y axis set up by QBarCategoryAxis
	QBarCategoryAxis* axisY;
	// x axis set up
	QCategoryAxis* axisX;

	// switch of green red light in order to build up signal timing plan chart
	bool m_isStartGreen; 

	// internal backup data for table
	std::unordered_multimap<int, std::pair<int, int>> m_groupTime;  // key: sigGpId; values: (first:onT ; second:offT)

	// keep track of the selected SigsetID ;if m_isConfirmed == false, could set the SigsetID combobox back to m_currentSigsetID
	int m_currentSigsetID;
	std::unordered_map<int, int> m_sigSetIndex; //key: index | value: sigSetId

	// get from DBaseNaviFunction ---> DJunctionModeFunction---> selected Junction
	int m_JctID;

	bool m_isConfirmed = true; // default in true coz no data changes yet

	// --- --- --- --- --- internal function --- --- --- --- --- 
	void CheckAndCreateSigCtrl();
	void CreateDefaultChart();
	bool CheckInputtedGreenTime();
	void ScrapeDownDataFromCoreIntoTable(int index);
	void AddSigSetItemintoCombobox();
	void setCurrentSelectedSigSet(int SigsetID);

public:
	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---
	DSignalTimingPlanCreateFunc();
	~DSignalTimingPlanCreateFunc();

	// --- --- --- --- --- override initFunctionHandler from I_Function --- --- --- --- ---
	void initFunctionHandler() override;

public slots:
	// --- --- --- --- --- Slots --- --- --- --- ---
	bool BuildUpChartContentFromTable();
	void DataChanged();
	void UpdateChartAndTableBySigSetSwitch(int index);
	void sigGpSelectionChanged();

	// --- --- --- --- --- Checking and creation Functions --- --- --- --- ---
	void checkAndSubmitChanges();
	// close func and the dialog
	void dialogFinished();
	// close func when sigset not found
	void CancelFunction();

public:
	// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---
	bool keyPressEvent(QKeyEvent* keyEvent) override;
};
