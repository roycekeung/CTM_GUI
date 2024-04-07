#include "DSignalTimingPlanCreateFunc.h"

// std lib
#include <unordered_set>
#include <sstream>

// DISCO_GUI lib
#include "GraphicsItems/DGraphicsScene.h"
#include "GraphicsItems/DPseudoJctLinkItem.h"
#include "PanelWidgets/Junction/DSignalTimingPlanDialogue.h"
#include "DTabToolBar.h"
#include "DBaseNaviFunction.h"
#include "Functions/Junction/DJunctionModeFunction.h"

// Qt lib
#include <QSpinBox>
#include <QVector>
#include <QList>
#include <QMessageBox>
// Qt Chart lib
#include <qtcharts/qbarseries>
#include <qtcharts/qbarset>
#include <qtcharts/qlegend>
#include <QtCharts/qlegendmarker.h>

// DISCO_Core lib
#include <Sim_Builder.h>
#include <Net_Container.h>
#include <Jct_Node.h>


// --- --- --- --- --- internal function --- --- --- --- --- 
void DSignalTimingPlanCreateFunc::CheckAndCreateSigCtrl() {

	std::unordered_set<int> tmp_SigGpIDs = this->ref_scn->get()->getNetwork().getJctNode(this->m_JctID).getSigGpIds();
	std::unordered_set<int> tmp_SigSetIDs = this->ref_scn->get()->getSignal().getSigSetIds();

	for (int SigSetID : tmp_SigSetIDs) {
		try {
			// coz return this->m_sigControllers.at(jctNodeId); would fail if sigCtrl hasnt been created, .at(xx) would fail finding the Jctnode
			// Wont catch error if SigCtrl do exist
			this->ref_scn->get()->getSignal().getSigSet(SigSetID).getSigCtrl(this->m_JctID);
			
			// set the true value of cycle time and offset from the core whereas the SigCtrl do exist
			this->m_DialogueWigdet->getUI().CycleTime_spinBox->setValue(this->ref_scn->get()->getSignal().getSigSet(SigSetID).getFGFC_cycleTime(this->m_JctID));
			this->m_DialogueWigdet->getUI().Offset_spinBox->setValue(this->ref_scn->get()->getSignal().getSigSet(SigSetID).getFGFC_offset(this->m_JctID));
		}
		catch (...) {
			//failure it means the corresponding junction and sigGroup is newly created so is about to create Defualt FGFC sigCtrl
			// coz CycleTime and Offset havent been recorded in core since sigCtrl doesnt even exist; thus use default val in ui
			this->ref_scn->get()->getSigEditor().sigCtrl_FGFC_create(
				SigSetID, 
				this->m_JctID, 
				this->m_DialogueWigdet->getCycleTime(),  // use default CycleTime that defined in ui
				this->m_DialogueWigdet->getOffset());  // use default OffsetTime that defined in ui
			
			// Noted : CycleTime and Offset could be various in diff SigSetID;; but all've been in default val according to ui if no sigCtrl
		}
	}
}


void DSignalTimingPlanCreateFunc::CreateDefaultChart() {

	if (!this->m_barChart) {
		// means chart havent been initiated yet
		
		// create empty chart
		this->m_barChart = new QChart();
		QFont titlename_font = this->m_barChart->titleFont();
		titlename_font.setPointSize(10);
		titlename_font.setBold(true);
		this->m_barChart->setTitleFont(titlename_font);
		this->m_barChart->setAnimationOptions(QChart::SeriesAnimations);   // aesthetic animation turn off since value change of spinbox would keep updating the chart
		this->m_barChart->setAnimationDuration(100);
		this->m_barChart->setTheme(QChart::ChartThemeBrownSand);
		// insert empty chart into view
		this->m_barChartView = new QChartView();
		this->m_barChartView->setChart(this->m_barChart);
		this->m_barChartView->setRenderHint(QPainter::Antialiasing);
		this->m_barChartView->setBackgroundBrush(QBrush(Qt::transparent));
		// add in to layout within dialog
		this->m_DialogueWigdet->getUI().horizontalLayoutOfTableAndChart->addWidget(this->m_barChartView);
		this->m_DialogueWigdet->getUI().horizontalLayoutOfTableAndChart->setStretch(0, 2);  // table, spinbox, show chart view button layout Stretch less
		this->m_DialogueWigdet->getUI().horizontalLayoutOfTableAndChart->setStretch(1, 3); // chartview layout Stretch more

		// build up the bar elements
		this->redtimeBar = new QBarSet(QString("red"), this->m_barChart);
		this->greentimeBar = new QBarSet(QString("green"), this->m_barChart);
		this->redtime2Bar = new QBarSet(QString("red"), this->m_barChart);
		this->greentime2Bar = new QBarSet(QString("green"), this->m_barChart);
		this->redtime3Bar = new QBarSet(QString("red"), this->m_barChart);
		this->greentime3Bar = new QBarSet(QString("green"), this->m_barChart);

		// setting the color of the bar
		this->redtimeBar->setColor(QColor(255, 0, 0));
		this->greentimeBar->setColor(QColor(0, 128, 0));
		this->redtime2Bar->setColor(QColor(255, 0, 0));
		this->greentime2Bar->setColor(QColor(0, 128, 0));
		this->redtime3Bar->setColor(QColor(255, 0, 0));
		this->greentime3Bar->setColor(QColor(0, 128, 0));

		// series for horizontalwise stacking bars
		this->barseries = new QHorizontalStackedBarSeries(this->m_barChart);
		// y axis set up by QBarCategoryAxis
		this->axisY = new QBarCategoryAxis(this->m_barChart);
		// x axis set up
		this->axisX = new QCategoryAxis(this->m_barChart);

	}
	else {
		// Chart had been set up more than one times

		//since barset dont have built-in clear function in qt; thus manually delete and recreate the QBarSet to avoid stacking up qlist value that rest from previous SigSetID GreenData Settting
		delete this->redtimeBar;
		delete this->greentimeBar;
		delete this->redtime2Bar;
		delete this->greentime2Bar;
		delete this->redtime3Bar;
		delete this->greentime3Bar;

		// build up the bar elements
		this->redtimeBar = new QBarSet(QString("red"), this->m_barChart);
		this->greentimeBar = new QBarSet(QString("green"), this->m_barChart);
		this->redtime2Bar = new QBarSet(QString("red"), this->m_barChart);
		this->greentime2Bar = new QBarSet(QString("green"), this->m_barChart);
		this->redtime3Bar = new QBarSet(QString("red"), this->m_barChart);
		this->greentime3Bar = new QBarSet(QString("green"), this->m_barChart);

		// setting the color of the bar
		this->redtimeBar->setColor(QColor(255, 0, 0));
		this->greentimeBar->setColor(QColor(0, 128, 0));
		this->redtime2Bar->setColor(QColor(255, 0, 0));
		this->greentime2Bar->setColor(QColor(0, 128, 0));
		this->redtime3Bar->setColor(QColor(255, 0, 0));
		this->greentime3Bar->setColor(QColor(0, 128, 0));

		// delete barseries ; Removes all bar sets from the series and permanently deletes them.
		delete this->barseries;
		// series for horizontalwise stacking bars
		this->barseries = new QHorizontalStackedBarSeries(this->m_barChart);

		// delete axisY and recreate 
		delete this->axisY;
		// y axis set up by QBarCategoryAxis
		this->axisY = new QBarCategoryAxis(this->m_barChart);

		// delete axisX ; since axisX dont have built-in clear function in qt; thus manually delete and recreate 
		delete this->axisX;
		// x axis set up
		this->axisX = new QCategoryAxis(this->m_barChart);
	}
}

bool DSignalTimingPlanCreateFunc::CheckInputtedGreenTime() {
	// get total row count
	int total_TableRowCount = this->m_DialogueWigdet->getUI().TimingIput_tableWidget->rowCount();
	int row;
	for (int cur_row = 0; cur_row < total_TableRowCount; cur_row++) {
		// getting GreenTime Cell 
		int GOnT = (static_cast<QSpinBox*>(this->m_DialogueWigdet->getUI().TimingIput_tableWidget->cellWidget(cur_row, 1)))->value();
		int GOffT = (static_cast<QSpinBox*>(this->m_DialogueWigdet->getUI().TimingIput_tableWidget->cellWidget(cur_row, 2)))->value();
		int G2OnT = (static_cast<QSpinBox*>(this->m_DialogueWigdet->getUI().TimingIput_tableWidget->cellWidget(cur_row, 4)))->value();
		int G2OffT = (static_cast<QSpinBox*>(this->m_DialogueWigdet->getUI().TimingIput_tableWidget->cellWidget(cur_row, 5)))->value();
		int cycleT = this->m_DialogueWigdet->getCycleTime();

		// check GreenOnTime timing overlap conflict
		if (GOnT == GOffT || G2OnT == G2OffT) {
			// do nth continue, no green overlap conflict coz only one green bar
		}
		else {
			//adjust the on/off time values
			if (GOffT < GOnT)
				GOffT += cycleT;
			if (G2OffT < G2OnT)
				G2OffT += cycleT;

			//check if they overlap
			if ((GOnT < G2OnT && G2OnT < GOffT)
				|| (G2OnT < GOnT && GOnT < G2OffT))
				return false;
		}
	}

	// true means InputtedGreenTime is within the valid range
	return true;
}

void DSignalTimingPlanCreateFunc::ScrapeDownDataFromCoreIntoTable(int index) {
	// clear once just in case the change of SigSetComboBox trigger this functions whereas it had contained the old gen data inf from previous SigSetID
	this->m_groupTime.clear();

	//get all the things
	auto& thisSigSet = this->ref_scn->get()->getSignal().getSigSet(this->m_sigSetIndex.at(index));
	auto& sigGps = this->ref_scn->get()->getNetwork().getJctNode(this->m_JctID).getSigGpIds();

	// scrape down the Cycle time val from core into UI widget shown
	int tmp_cycletime = thisSigSet.getFGFC_cycleTime(this->m_JctID);
	this->m_DialogueWigdet->getUI().CycleTime_spinBox->setValue(tmp_cycletime);
	// scrape down the offset time val from core into UI widget shown
	int tmp_offsettime = thisSigSet.getFGFC_offset(this->m_JctID);
	this->m_DialogueWigdet->getUI().Offset_spinBox->setValue(tmp_offsettime);

	// clear off all rows in table; preparation for data refilled from core regards of diff SigsetID
	this->m_DialogueWigdet->ClearAllContentInTable();

	// get the vector of veh sig group or pessenger sig group, those conclude groupID, Green on and off time
	// if ites newly created FGFC sigCtrl, vector groupTimes would be empty 
	std::vector<DISCO2_API::I_SigCtrl::groupTime> groupTimes = thisSigSet.getFGFC_sigGroupTimes(this->m_JctID);

	for (auto& groupTime : groupTimes) {
		// record data from core to internal backup to check single green vecotr or multiple
		this->m_groupTime.emplace(groupTime.sigGpId, std::make_pair(groupTime.onT, groupTime.offT));

		if (this->m_groupTime.count(groupTime.sigGpId) == 1) {
			// data from core write into table
			this->m_DialogueWigdet->addRowToTable(
				this,
				groupTime.sigGpId,
				groupTime.onT,
				groupTime.offT,
				-1,
				-1);
		}
		else if (this->m_groupTime.count(groupTime.sigGpId) > 1) {
			// sigsetID shows up again, thus 2 green vector of green bars
			this->m_DialogueWigdet->OverwriteRowToTableRegardsToSigGpID(
				groupTime.sigGpId,
				this->m_groupTime.find(groupTime.sigGpId)->second.first, // old onT had recorded from core
				this->m_groupTime.find(groupTime.sigGpId)->second.second, // old offT had recorded from core
				groupTime.onT,  // new onT just read from core
				groupTime.offT);// new offT just read from core

		}
	}

	//put in dummies iff created sigGp is not in the sigCtrl
	for (auto& sigGpId : sigGps) {
		if (!this->m_groupTime.count(sigGpId))
			this->m_DialogueWigdet->addRowToTable(this, sigGpId, -1, -1, -1, -1);
	}

	// green, offset, cycle time data was just recorded down so its intact and unmodified
	// since offset, cycle time spin box siganl valuechange is connect to this function slot DataChanged which will presume u modified data already(make this->m_isConfirmed = false)
	// so to rectify it back here
	this->m_isConfirmed = true;

}


void DSignalTimingPlanCreateFunc::AddSigSetItemintoCombobox() {
	if (this->ref_scn->get()->getSignal().getSigSetIds().empty()) {
		return;
	}

	int index = 0;
	for (int sigSetID : this->ref_scn->get()->getSignal().getSigSetIds()) {
		//build the name text
		std::stringstream text;
		text << sigSetID << ' ';
		text << this->ref_scn->get()->getSignal().getSigSet(sigSetID).getVisInfo().getName();

		this->m_DialogueWigdet->getUI().SigsetcomboBox->addItem(text.str().c_str());
		this->m_sigSetIndex.emplace(index, sigSetID);

		if (this->ref_scn->get()->getSimBuilder().getSigSetUsing() == sigSetID)
			this->m_DialogueWigdet->setSigSetComboBoxIndex(index);

		index++;
	}

	// set the current sigset using in core into the Combobox
	this->m_DialogueWigdet->setSigSetComboBoxIndex(0);
	this->m_currentSigsetID = this->ref_scn->get()->getSimBuilder().getSigSetUsing();
}


void DSignalTimingPlanCreateFunc::setCurrentSelectedSigSet(int SigsetID) {
	// set back the last selected Signal_setid 
	this->ref_scn->get()->getSimBuilder().useSigSet(SigsetID);
}



// --- --- --- --- --- Constructor Destructor --- --- --- --- ---
DSignalTimingPlanCreateFunc::DSignalTimingPlanCreateFunc():QObject(nullptr){

}

DSignalTimingPlanCreateFunc::~DSignalTimingPlanCreateFunc(){

}


// --- --- --- --- --- override initFunctionHandler from I_Function --- --- --- --- ---
void DSignalTimingPlanCreateFunc::initFunctionHandler() {
	// disable the tabtoolbar first, no other button can be clicked
	this->ref_tabToolBar->setEnabled(false);

	// check if sigset exist, if no then it probably was just newly created scene so no demand nor signal set yet
	if (this->ref_scn->get()->getSignal().getSigSetIds().empty()) {
		// print out corresponding error
		QMessageBox ErrormessageBox;
		QString Error_name = QString("No Signal Set is found, please create one first");
		ErrormessageBox.setFixedSize(500, 200);
		ErrormessageBox.setStandardButtons(QMessageBox::Cancel);
		ErrormessageBox.critical(nullptr, "Error", Error_name);
		// no further action is applied onto the standarad button
		// close the function and enable the tab toolbar
		CancelFunction();
	}
	else {
		// only if sigset is not empty, then it keeps on the following action of creating signak timing plan
		this->m_DialogueWigdet = new DSignalTimingPlanDialogue();
		this->m_DialogueWigdet->setupCreateButtons(this);

		// Set row highlight when selected
		this->m_DialogueWigdet->getUI().TimingIput_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
		// turn off the row header of qtablewidget 
		this->m_DialogueWigdet->getUI().TimingIput_tableWidget->verticalHeader()->hide();
		// shows tool tips
		this->m_DialogueWigdet->setAttribute(Qt::WA_AlwaysShowToolTips, true);
		//QHeaderView will automatically resize the section to fill the available space horizontally, fixed vertically. 
		this->m_DialogueWigdet->getUI().TimingIput_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
		this->m_DialogueWigdet->getUI().TimingIput_tableWidget->verticalHeader()->setMinimumSectionSize(0);
		this->m_DialogueWigdet->getUI().TimingIput_tableWidget->verticalHeader()->setOffset(0);

		// turn off the built-in help question mark button
		this->m_DialogueWigdet->setWindowFlags(this->m_DialogueWigdet->windowFlags() & ~Qt::WindowContextHelpButtonHint);


		// setup table widget and read SigSet intfo
		AddSigSetItemintoCombobox();

		this->m_JctID = this->ref_baseNaviFunction->getJctModeFunction()->getCurrentJctId();

		// if its loaded SigSet so should have SigCtrl already then this func wont act on create new SigCtrl; run after selected junction is defined
		CheckAndCreateSigCtrl();
		// run after CheckAndCreateSigCtr is defined to avoid empty SigCtr
		ScrapeDownDataFromCoreIntoTable(this->m_DialogueWigdet->getSigSetComboBoxIndex());

		// run after ScrapeDownDataFromCoreIntoTable is defined to avoid empty table sourcing
		BuildUpChartContentFromTable();

		// connect custom signal slot responds
		this->m_DialogueWigdet->connectSigSetComboBox(this);
		this->m_DialogueWigdet->connectDataChanged(this);

		// coz changes have committed  could hide the button temporary unless further changes made
		this->m_DialogueWigdet->setref_confirmButtonEnabled(false);

		// run dialog
		this->m_DialogueWigdet->exec();

	}
}

// --- --- --- --- --- Slots --- --- --- --- ---
bool DSignalTimingPlanCreateFunc::BuildUpChartContentFromTable() {

	// coz this function would also connect to UpdateChart_Button cliked
	// so gotta be inner check the green requirement before this build up of chartContent
	if (CheckInputtedGreenTime() == false) {
		this->m_DialogueWigdet->failMessage({ "green time inputted is invalid, have overlapping comflicts" });
		// fail on building chart; so no updating the chart view and contents
		return false;
	}
	else {
		// delete the failMessage, it could probably last from the previous green time overlapping conflicts
		this->m_DialogueWigdet->failMessage({ "" });
	}

	// initialize chart anyway no matter which action triggered either the validation of following green input or switch of SigSetID
	CreateDefaultChart();

	// tmp pre def of Y-axis categories and bar contents list
	QVector<QString >  categoriesVec{};
	QVector<qreal > redtimeVec{};
	QVector<qreal > greentimeVec{};
	QVector<qreal > redtime2Vec{};
	QVector<qreal > greentime2Vec{};
	QVector<qreal > redtime3Vec{};
	QVector<qreal > greentime3Vec{};

	std::vector<QVector<qreal >> barseriesVec{ redtimeVec , greentimeVec , redtime2Vec , greentime2Vec , redtime3Vec , greentime3Vec };


	// to record down every cuting time of each green bars start; used to draw grid line on chart later
	std::set<int> gridline_cut_pos;

	int tmp_cycletime = this->m_DialogueWigdet->getCycleTime();

	// get total row count
	int total_TableRowCount = this->m_DialogueWigdet->getUI().TimingIput_tableWidget->rowCount();

	// BuildUpChartContentFromTable; data source from the Green Input table
	for (int cur_row = 0; cur_row < total_TableRowCount; cur_row++) {
		// getting each QTableWidgetItem items row by row
		QSpinBox* SigGpIDCell = static_cast<QSpinBox*>(this->m_DialogueWigdet->getUI().TimingIput_tableWidget->cellWidget(cur_row, 0));

		// getting GreenTime Cell 
		int GOnT = (static_cast<QSpinBox*>(this->m_DialogueWigdet->getUI().TimingIput_tableWidget->cellWidget(cur_row, 1)))->value();
		int GOffT = (static_cast<QSpinBox*>(this->m_DialogueWigdet->getUI().TimingIput_tableWidget->cellWidget(cur_row, 2)))->value();
		int G2OnT = (static_cast<QSpinBox*>(this->m_DialogueWigdet->getUI().TimingIput_tableWidget->cellWidget(cur_row, 4)))->value();
		int G2OffT = (static_cast<QSpinBox*>(this->m_DialogueWigdet->getUI().TimingIput_tableWidget->cellWidget(cur_row, 5)))->value();

		std::unordered_set<int> cut_pts{};
		if (GOnT >= 0 && GOffT >= 0 && GOnT != GOffT) {
			if (GOnT > 0)
				cut_pts.insert(GOnT);
			cut_pts.insert(GOffT);
			// remove that gridline tick
			gridline_cut_pos.insert(GOnT);
		}
		if (G2OnT >= 0 && G2OffT >= 0 && G2OnT != G2OffT) {
			if (G2OnT > 0)
				cut_pts.insert(G2OnT);
			cut_pts.insert(G2OffT);
			// remove that gridline tick
			gridline_cut_pos.insert(G2OnT);
		}
		
		//special case both start end at same point
		if (GOffT == G2OnT)
			cut_pts.erase(G2OnT);
		if (G2OffT == GOnT)
			cut_pts.erase(GOnT);

		//determine which starts first
		this->m_isStartGreen = ((GOnT >= 0 && GOffT >= 0 && (GOffT <= GOnT || GOnT == 0))  
			|| (G2OnT >= 0 && G2OffT >= 0 && (G2OffT <= G2OnT || G2OnT == 0)));

		// used for tracking individual bar length
		int BarLengthBeenBuilt = 0;
		// unmber of bars that has been used
		int numBarUsed = 0;
		for (int t = 0; t <= tmp_cycletime; ++t) {
			//all green
			if ((GOnT >= 0 && GOffT >= 0 && GOffT == GOnT) || (G2OnT >= 0 && G2OffT >= 0 && G2OffT == G2OnT)) {
				// add 0 to redtimeVec bar
				barseriesVec.at(numBarUsed).append(0);
				// shift 1 index from redtimeVec to greentimeVec
				++numBarUsed;
				// add time to greentimeVec bar
				barseriesVec.at(numBarUsed).append(tmp_cycletime);
				++numBarUsed;
				// update the length (time) of bar that has been used
				BarLengthBeenBuilt = t;
				break;
			}
			// all red
			else if (cut_pts.empty()) {
				// add time to redtimeVec bar
				barseriesVec.at(numBarUsed).append(tmp_cycletime);
				++numBarUsed;
				break;
			}

			//start is green
			if (this->m_isStartGreen && numBarUsed == 0) {
				// add 0 to redtimeVec bar
				barseriesVec.at(numBarUsed).append(0);
				// shift 1 index from redtimeVec to greentimeVec
				++numBarUsed;
				// update the length (time) of bar that has been used
				BarLengthBeenBuilt = t;
			}

			//do the switching
			if (cut_pts.count(t)) {
				// add time to greentimeVec bar
				barseriesVec.at(numBarUsed).append(t - BarLengthBeenBuilt);
				++numBarUsed;
				// update the length (time) of bar that has been used
				BarLengthBeenBuilt = t;
			}
			else if (t == tmp_cycletime) {
				// enclose the bars stacking by either red bar or green bar
				barseriesVec.at(numBarUsed).append(tmp_cycletime - BarLengthBeenBuilt);
				++numBarUsed;
				// update the length (time) of bar that has been used
				BarLengthBeenBuilt = t;
			}
		}

		if (numBarUsed < (int)barseriesVec.size()) {
			// to enclose the bars stacking by 0 empty bar
			for (int restNumOfUnusedBars = numBarUsed; restNumOfUnusedBars < (int)barseriesVec.size(); ++restNumOfUnusedBars) {
				// adding 0 time to the rest Of Unused bars
				barseriesVec.at(restNumOfUnusedBars).append(0);
			}

		}

		// add in the sigGroupID to axis categories
		categoriesVec.append(QString("SigGp %1").arg(SigGpIDCell->value()));

	}

	// --- --- --- --- --- --- --- Method 1: time elapse --- --- --- --- --- --- ---
	// reverse the data list and categories naming title list in which means the reverse representation in y axis direction
	std::reverse(categoriesVec.begin(), categoriesVec.end());
	for (int numBars = 0; numBars < (int)barseriesVec.size(); ++numBars) {
		std::reverse(barseriesVec.at(numBars).begin(), barseriesVec.at(numBars).end());
	}

	// convert to QList prepared for inputtig into barset and axis
	QList<QString> categoriesList = categoriesVec.toList();
	QList<qreal> redtimeList = barseriesVec.at(0).toList();
	QList<qreal> greentimeList = barseriesVec.at(1).toList();
	QList<qreal> redtime2List = barseriesVec.at(2).toList();
	QList<qreal> greentime2List = barseriesVec.at(3).toList();
	QList<qreal> redtime3List = barseriesVec.at(4).toList();
	QList<qreal> greentime3List = barseriesVec.at(5).toList();


	// add tmp list into barset
	this->redtimeBar->append(redtimeList);
	this->greentimeBar->append(greentimeList);
	this->redtime2Bar->append(redtime2List);
	this->greentime2Bar->append(greentime2List);
	this->redtime3Bar->append(redtime3List);
	this->greentime3Bar->append(greentime3List);

	//// series for horizontalwise stacking bars
	this->barseries->append(this->redtimeBar);
	this->barseries->append(this->greentimeBar);
	this->barseries->append(this->redtime2Bar);
	this->barseries->append(this->greentime2Bar);
	this->barseries->append(this->redtime3Bar);
	this->barseries->append(this->greentime3Bar);

	this->barseries->setLabelsVisible(true);
	this->barseries->setLabelsFormat(QStringLiteral("@value"));
	this->barseries->setLabelsPosition(QAbstractBarSeries::LabelsCenter);

	// clear off leftover from previous green data of sigsetID
	this->m_barChart->removeAllSeries();
	// insert the horizontal stacked bar series into the chart first; so that legend wont be empty again 
	this->m_barChart->addSeries(this->barseries);
	this->m_barChart->update();

	// --- --- --- --- --- --- chart title set up--- --- --- --- --- --- 
	this->m_barChart->setTitle(QString("Signal Timing Plan for Junction Node: %1").arg(this->m_JctID));

	// --- --- --- --- --- --- y axis set up by QBarCategoryAxis--- --- --- --- --- --- 
	//this->axisY->clear();
	this->axisY->append(categoriesList);
	this->axisY->setGridLineVisible(false);
	this->axisY->setShadesVisible(false);
	this->m_barChart->createDefaultAxes();
	this->m_barChart->setAxisY(this->axisY, this->barseries);

	// --- --- --- --- --- --- x axis set up by QBarCategoryAxis--- --- --- --- --- --- 
	this->axisX->append(QString("%1").arg(0), 0);  // start of tick
	for (int tick_pos : gridline_cut_pos) {
		this->axisX->append(QString("%1").arg(tick_pos), tick_pos);
	}//// include the end of the tick but without the start of tick at 0
	this->axisX->append(QString("%1").arg(tmp_cycletime), tmp_cycletime);  // end of tick
	// pen defined for grid cut line
	QPen grid_pen = QPen();
	grid_pen.setWidthF(1);
	grid_pen.setCosmetic(true);
	grid_pen.setDashOffset(3.5);
	grid_pen.setColor(QColor(220, 220, 220));
	this->axisX->setGridLinePen(grid_pen);
	// x axis set up into chart
	this->axisX->setRange(0, tmp_cycletime);
	this->axisX->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
	this->axisX->setTitleText(QString("Time (sec)"));
	this->barseries->attachAxis(this->axisX);
	this->m_barChart->setAxisX(this->axisX);

	// --- --- --- --- --- --- legends set up--- --- --- --- --- --- 
	//  this is to hide part of the legends
	this->m_barChart->legend()->markers(this->barseries)[2]->setVisible(false); // redtime2Bar
	this->m_barChart->legend()->markers(this->barseries)[3]->setVisible(false);    // greentime2Bar  
	this->m_barChart->legend()->markers(this->barseries)[4]->setVisible(false); // redtime3Bar
	this->m_barChart->legend()->markers(this->barseries)[5]->setVisible(false);    // greentime3Bar  
	// set legend to bottom pos
	this->m_barChart->legend()->setAlignment(Qt::AlignBottom);

	// successfully built the chart
	return true;
}

void DSignalTimingPlanCreateFunc::DataChanged() {
	this->m_isConfirmed = false;
}

void DSignalTimingPlanCreateFunc::UpdateChartAndTableBySigSetSwitch(int index) {
	if (this->m_barChart) {
		// Chart had been set up more than one times
		if (m_isConfirmed == true) {
			this->m_currentSigsetID = this->m_sigSetIndex.at(index);

			// can be swicthed; its been submited changes to core or dont even have changes in green time input
			ScrapeDownDataFromCoreIntoTable(index);

			if (BuildUpChartContentFromTable() == true) {
				// clear all fail message; errors r cleared
				this->m_DialogueWigdet->failMessage({ "" });

				// coz changes have committed  could hide the button temporary unless further changes made
				this->m_DialogueWigdet->setref_confirmButtonEnabled(false);
			}

		}
		else {
			// set the SigsetID combobox back to m_currentSigsetID
			int lastIndex = 0;
			for(auto& entry : this->m_sigSetIndex)
				if (entry.second == this->m_currentSigsetID) {
					lastIndex = entry.first;
					break;
				}

			this->m_DialogueWigdet->setSigSetComboBoxIndex(lastIndex);
			this->m_DialogueWigdet->failMessage({ "Please submit the changes first before switch of SigSetID" });
		}

	}
}

void DSignalTimingPlanCreateFunc::sigGpSelectionChanged() {
	auto& conenctors = this->ref_graphicsScene->getPseudoJctLinksInJct(this->m_JctID);
	const auto& jctNode = this->ref_scn->get()->getNetwork().getJctNode(this->m_JctID);
	//clear selection
	for (auto& connector : conenctors)
		connector.second->setSelected(false);

	//find the row
	for (auto& row : this->m_DialogueWigdet->getUI().TimingIput_tableWidget->selectionModel()->selectedRows()) {
		int sigGpId = ((QSpinBox*)(this->m_DialogueWigdet->getUI().TimingIput_tableWidget->cellWidget(row.row(), 0)))->value();

		//select the jLinks
		if (jctNode.getSigGpIds().count(sigGpId))
			for (auto& jLinkId : jctNode.getSigGpJLinkIds(sigGpId))
				if (conenctors.count(jLinkId))
					conenctors.at(jLinkId)->setSelected(true);
	}
}

// --- --- --- --- --- Checking and creation Functions --- --- --- --- ---
void DSignalTimingPlanCreateFunc::checkAndSubmitChanges() {
	// anyway , update the chart view visually first , inrelated to saving data into core

	// so gotta be inner check the green requirement before this build up of chartContent
	if (CheckInputtedGreenTime() == false) {
		this->m_DialogueWigdet->failMessage({ "green time inputted is invalid, have overlapping comflicts" });
		// fail on building chart; so no updating the chart view and contents and skip saving data into core process
		return;
	}
	else {
		// delete the failMessage, it could probably last from the previous green time overlapping conflicts
		this->m_DialogueWigdet->failMessage({ "" });
	}


	try {
		int sigSetId = this->m_sigSetIndex.at(this->m_DialogueWigdet->getSigSetComboBoxIndex());

		// clear off all this->m_sigGpTimes std::vector<groupTime> 
		this->ref_scn->get()->getSigEditor().sigCtrl_FGFC_removeAllGpTime(sigSetId, this->m_JctID);
		
		// --- --- --- --- --- --- saving cycle time into core --- --- --- --- --- --- 
		this->ref_scn->get()->getSigEditor().sigCtrl_FGFC_setCycleTime(
			sigSetId, this->m_JctID, this->m_DialogueWigdet->getCycleTime());
		// --- --- --- --- --- --- saving offset time into core --- --- --- --- --- --- 
		this->ref_scn->get()->getSigEditor().sigCtrl_FGFC_setOffSet(
			sigSetId, this->m_JctID, this->m_DialogueWigdet->getOffset());
		// --- --- --- --- --- --- saving green time into core --- --- --- --- --- --- 
		// get total row count in table
		int total_TableRowCount = this->m_DialogueWigdet->getUI().TimingIput_tableWidget->rowCount();
		// get data source from the Green Input table
		for (int cur_row = 0; cur_row < total_TableRowCount; cur_row++) {
			// getting each QTableWidgetItem items row by row
			QSpinBox* SigGpIDCell = static_cast<QSpinBox*>(this->m_DialogueWigdet->getUI().TimingIput_tableWidget->cellWidget(cur_row, 0));

			// getting GreenTime Cell 
			int GOnT = (static_cast<QSpinBox*>(this->m_DialogueWigdet->getUI().TimingIput_tableWidget->cellWidget(cur_row, 1)))->value();
			int GOffT = (static_cast<QSpinBox*>(this->m_DialogueWigdet->getUI().TimingIput_tableWidget->cellWidget(cur_row, 2)))->value();
			int G2OnT = (static_cast<QSpinBox*>(this->m_DialogueWigdet->getUI().TimingIput_tableWidget->cellWidget(cur_row, 4)))->value();
			int G2OffT = (static_cast<QSpinBox*>(this->m_DialogueWigdet->getUI().TimingIput_tableWidget->cellWidget(cur_row, 5)))->value();

			if (GOnT >= 0 && GOffT >= 0 && GOnT != GOffT) {
				this->ref_scn->get()->getSigEditor().sigCtrl_FGFC_addGpTime(
					sigSetId,
					this->m_JctID,
					SigGpIDCell->value(),
					GOnT,
					GOffT);
			}
			if (G2OnT >= 0 && G2OffT >= 0 && G2OnT != G2OffT) {
				this->ref_scn->get()->getSigEditor().sigCtrl_FGFC_addGpTime(
					sigSetId,
					this->m_JctID, 
					SigGpIDCell->value(),
					G2OnT,
					G2OffT);
			}
		}

		// if successfully submit changes to the core then m_isConfirmed would chnage to true
		this->m_isConfirmed = true;

		// clear all fail message; errors r cleared
		this->m_DialogueWigdet->failMessage({ "" });

		// coz changes have committed  could hide the button temporary unless further changes made
		this->m_DialogueWigdet->setref_confirmButtonEnabled(false);

	}
	catch(std::exception & e) {
		//print fail msg in the widget
		this->m_DialogueWigdet->failMessage({ e.what() });
	}
}


// --- --- --- --- --- Slots for Panel --- --- --- --- ---
// close func and the dialog
void DSignalTimingPlanCreateFunc::dialogFinished() {
	// the tabtoolbar return normal 
	this->ref_tabToolBar->setEnabled(true);

	// delete Chart related ptr
	delete this->m_barChart;
	delete this->m_barChartView;
	// those parent of QBarSet, QHorizontalStackedBarSeries, QBarCategoryAxis and QCategoryAxis r m_barChart so it will supposed to be also deleted followed by its parent

	// save the last selected row to be the current setID
	this->setCurrentSelectedSigSet(this->m_sigSetIndex.at(this->m_DialogueWigdet->getSigSetComboBoxIndex()));
	this->m_DialogueWigdet->close();
	// end this function
	this->closeFunction();
}

// close func when sigset not found
void DSignalTimingPlanCreateFunc::CancelFunction() {
	// the tabtoolbar return normal 
	this->ref_tabToolBar->setEnabled(true);

	// end this function
	this->closeFunction();
}

// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---
bool DSignalTimingPlanCreateFunc::keyPressEvent(QKeyEvent* keyEvent) {
	if (keyEvent->key() == Qt::Key::Key_Escape) {
		// close func and the dialog
		dialogFinished();

		return true;
	}
	// continue to handle other events
	return false;
}


