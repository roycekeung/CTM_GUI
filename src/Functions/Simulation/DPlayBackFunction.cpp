#include "DPlayBackFunction.h"

#include "DFunctionHandler.h"
#include "DGraphicsScene.h"
#include "DDockWidgetHandler.h"
#include "DRunManager.h"
#include "DTabToolBar.h"
#include "DMainWindow.h"
#include "Functions/DBaseNaviFunction.h"
#include "Functions/Simulation/DGenDelayPlotFunction.h"
#include "Functions/Simulation/DGenQueueLengthFunction.h"
#include "PanelWidgets/Simulation/DPlayBackBar.h"
#include "PanelWidgets/Simulation/DPlayBackOptionsPanel.h"
#include "GraphicsItems/DCellItem.h"
#include "GraphicsItems/DDemandSinkCellItem.h"

#include <QDockWidget>

#include "Rec_All.h"

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DPlayBackFunction::DPlayBackFunction(size_t index) : QTimeLine(), I_Function(), m_index(index), m_gradientList() {
	this->setCurveShape(QTimeLine::LinearCurve);
}

DPlayBackFunction::~DPlayBackFunction() {
}

void DPlayBackFunction::initFunctionHandler() {
	//get the rec
	this->ref_runRec = this->ref_runManager->getRunRec(m_index);
	this->ref_rec = dynamic_cast<DISCO2_API::Rec_All*>(this->ref_runRec->rec);
	if (this->ref_runRec == nullptr || this->ref_rec == nullptr)
		this->cancel();

	//disable other stuff
	this->ref_baseNaviFunction->setIgnoreEvents(true);
	this->ref_tabToolBar->setPlaybackTab(true);

	//Prep graphics scene
	this->ref_graphicsScene->setShowAllJct(false);
	for (auto jctId : this->ref_scn->get()->getNetwork().getJctIds()) {
		this->ref_graphicsScene->addAllJLinkItemInJct(jctId);
		this->ref_graphicsScene->addAllDPseudoJLinkItemInJct(jctId);
		this->ref_graphicsScene->setShowDetailJctLinkView(jctId, !this->ref_runRec->sigRec.count(jctId));
	}

	//setup the panel widget
	this->initGradients();
	this->populateColorCache(0);
	this->addOptionsPanel();
	this->addTimeBar();
	
	//setup the timeline
	this->setStartFrame(0);
	this->setEndFrame(this->ref_rec->getAllRec().size() - 1);
	this->m_timeBar->setSpeed(1);
	this->inputSpeedChanged(1);

	//connect
	QObject::connect(this, &QTimeLine::frameChanged, this, &DPlayBackFunction::currentTimeChanged);
	QObject::connect(this->ref_tabToolBar->getExitPlayBackButton(), &QPushButton::clicked, this, &DPlayBackFunction::cancel);
	QObject::connect(this->ref_tabToolBar->getPlaybackOptionsButton(), &QPushButton::clicked, this, &DPlayBackFunction::addOptionsPanel);
	QObject::connect(this->ref_tabToolBar->getPlaybackTimeBarButton(), &QPushButton::clicked, this, &DPlayBackFunction::addTimeBar);
	QObject::connect(this->ref_tabToolBar->getPlaybackGenDelayPlotButton(), &QPushButton::clicked, this, &DPlayBackFunction::delayPlotClicked);
	QObject::connect(this->ref_tabToolBar->getPlaybackGenQueuePlotButton(), &QPushButton::clicked, this, &DPlayBackFunction::queuePlotClicked);
}

// --- --- --- --- --- Slots for Running --- --- --- --- ---

void DPlayBackFunction::cancel() {
	//reset normal graphics scene
	this->ref_graphicsScene->setShowAllJct(true);
	for (auto jctId : this->ref_scn->get()->getNetwork().getJctIds()) {
		this->ref_graphicsScene->removeAllJLinkItemInJct(jctId);
		this->ref_graphicsScene->removeAllDPseudoJLinkItemInJct(jctId);
		this->ref_graphicsScene->setShowDetailJctLinkView(jctId, !this->ref_runRec->sigRec.count(jctId));
	}
	this->ref_graphicsScene->resetAllCellColor();
	this->ref_graphicsScene->resetAllCellText();
	//remove the panel
	this->ref_dockWidgetHandler->removeAndDeleteRightDockWidget();
	this->ref_dockWidgetHandler->removeAndDeleteFloatDockWidget(this->m_timeBar);
	//re-enable stuff
	this->ref_baseNaviFunction->setIgnoreEvents(false);
	this->ref_tabToolBar->setPlaybackTab(false);

	this->closeFunction();
}

void DPlayBackFunction::inputTimeStepChanged(int value) {
	int currStep = value;
	int64_t currTime = (int64_t)value * this->ref_rec->getTimeStepSize() + this->ref_rec->getStartTime();

	if (this->state() == QTimeLine::State::Paused ||
		(this->state() == QTimeLine::State::Running && std::abs(currStep - this->currentFrame()) > 10)) {					//allow them a little bit of leeway

		if (this->currentFrame() != currStep)
			this->setCurrentTime(this->m_currentRate * currStep);

		if (this->m_timeBar) {	//may desync
			this->m_timeBar->setCurrentTime(currTime);
		}
	}
}

void DPlayBackFunction::inputTimeChanged(double value) {
	int currStep = (std::round(value * 1000) - this->ref_rec->getStartTime()) / this->ref_rec->getTimeStepSize();
	int64_t currTime = (int64_t)currStep * this->ref_rec->getTimeStepSize() + this->ref_rec->getStartTime();

	if (this->state() == QTimeLine::State::Paused ||
		(this->state() == QTimeLine::State::Running && std::abs(currStep - this->currentFrame()) > 10)) {					//allow them a little bit of leeway
		
		if (this->currentFrame() != currStep)
			this->setCurrentTime(this->m_currentRate * currStep);

		if (this->m_timeBar) {	//may desync
			this->m_timeBar->setCurrentTime(currTime);
		}
	}
}

void DPlayBackFunction::inputSpeedChanged(int value) {
	bool wasRunning = (this->state() == QTimeLine::Running);
	if (wasRunning)
		this->pause();
	int startFrame = this->currentFrame();
	this->m_currentRate = (1000 / value);
	this->setDuration(1000 / value * ((this->ref_rec->getEndTime() - this->ref_rec->getStartTime())/this->ref_rec->getTimeStepSize()));
	this->setCurrentTime(this->m_currentRate * startFrame);
	if (wasRunning)
		this->resume();
}

void DPlayBackFunction::currentTimeChanged(int value) {
	//convert the time
	int64_t currTime = (int64_t)value * this->ref_rec->getTimeStepSize() + this->ref_rec->getStartTime();

	using namespace DISCO2_API;
	auto& net = this->ref_scn->get()->getNetwork();

	//show JLink
	for (auto& jctEntry : this->ref_runRec->sigRec) {
		auto& jctNode = net.getJctNode(jctEntry.first);
		for (auto& sigGpEntry : jctEntry.second)
			if (jctNode.getSigGpIds().count(sigGpEntry.first)) {
				//on
				for (auto& t : sigGpEntry.second.first)
					if (t == currTime) {
						for (auto& jLinkId : jctNode.getSigGpJLinkIds(sigGpEntry.first))
							this->ref_graphicsScene->setShowDetailJctLinkView(jctEntry.first, jLinkId, true);
						break;
					}
				//off
				for (auto& t : sigGpEntry.second.second)
					if (t == currTime) {
						for (auto& jLinkId : jctNode.getSigGpJLinkIds(sigGpEntry.first))
							this->ref_graphicsScene->setShowDetailJctLinkView(jctEntry.first, jLinkId, false);
						break;
					}
			}
	}

	//do the cells
	for (auto& stuff : this->ref_rec->getAllRec().at(value)) {
		DCellItem* cellItem = this->ref_graphicsScene->getDCellItem(stuff.first);
		DDemandSinkCellItem* dmdCellItem = this->ref_graphicsScene->getDmdSnkCellItem(stuff.first);
		if (cellItem) {
			//color
			cellItem->setUseCustomColor(true, this->m_colorCache.at(
				std::get<0>(stuff.second) * 100 / net.getCell(stuff.first)->getStorageCapacity()));
			//text
			switch (this->m_currentTextIndex) {
			case 0:
			default:
				cellItem->setUseText(false);
				break;
			case 1:	//plain value
				cellItem->setUseText(true, QString::number(std::get<0>(stuff.second), 'f', 3));
				break;
			case 2:	//percentage
				cellItem->setUseText(true, 
					(QString::number(std::get<0>(stuff.second) * 100 
						/ net.getCell(stuff.first)->getStorageCapacity(), 'f', 1).toStdString() + "%").c_str());
				break;
			}
			//draw
			cellItem->update();
		}
		else if (dmdCellItem && dmdCellItem->getTypeOfCell() == DDemandSinkCellItem::CellType::Demand) {
			//color
			dmdCellItem->setUseCustomColor(true, this->m_colorCache.at(
				std::min(100, (int)(std::get<0>(stuff.second)))));
			//text
			if (this->m_currentTextIndex)
				dmdCellItem->setUseText(true, QString::number(std::get<0>(stuff.second), 'f', 3));
			else
				dmdCellItem->setUseText(false);
			//draw
			dmdCellItem->update();
		}
	}

	//deal with time Bar
	if (this->m_timeBar)
		this->m_timeBar->setCurrentTime(currTime);
}

void DPlayBackFunction::populateColorCache(int index) {
	this->m_currentColorIndex = index;
	this->m_colorCache.clear();

	QLinearGradient gradient = this->m_gradientList.at(index);

	int i = 0;
	bool first = true;
	int thisStart;
	qreal r1, g1, b1;
	for (auto& stop : gradient.stops()) {
		//this ugly bit cause itr doesn't work
		if (first) {
			first = false;
			thisStart = stop.first * 100;
			r1 = stop.second.redF();
			g1 = stop.second.greenF();
			b1 = stop.second.blueF();
			continue;
		}

		int thisEnd = stop.first * 100;
		qreal r2 = stop.second.redF();
		qreal g2 = stop.second.greenF();
		qreal b2 = stop.second.blueF();

		int diff = thisEnd - thisStart;
		while (i <= thisEnd) {
			double thisFraction = ((double)i - thisStart) / diff;
			QColor temp;
			temp.setRgbF(
				thisFraction * (r2-r1) + r1,
				thisFraction * (g2-g1) + g1,
				thisFraction * (b2-b1) + b1);
			this->m_colorCache.emplace(i, std::move(temp));
			++i;
		}

		//swap everthing
		thisStart = thisEnd;
		r1 = r2;
		g1 = g2;
		b1 = b2;
	}

	assert(this->m_colorCache.size() == 101);
}

void DPlayBackFunction::textOptionChanged(int index) {
	this->m_currentTextIndex = index;
}

void DPlayBackFunction::pause() {
	this->setPaused(true);
}

void DPlayBackFunction::startOrResume() {
	if (this->state() == QTimeLine::Paused)
		this->resume();
	else
		this->start();
}

void DPlayBackFunction::addOptionsPanel() {
	this->m_optionsPanel = new DPlayBackOptionsPanel{ this->m_gradientList };
	this->ref_dockWidgetHandler->setRightDockWidget(this->m_optionsPanel, "Playback Options", true);
	//curret values
	this->m_optionsPanel->getStyleComboBox()->setCurrentIndex(this->m_currentColorIndex);
	this->m_optionsPanel->getCellLabelComboBox()->setCurrentIndex(this->m_currentTextIndex);
	//connect from options panel
	QObject::connect(this->m_optionsPanel->getStyleComboBox(), QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DPlayBackFunction::populateColorCache);
	QObject::connect(this->m_optionsPanel->getCellLabelComboBox(), QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DPlayBackFunction::textOptionChanged);
	QObject::connect(this->m_optionsPanel->getCloseButton(), &QPushButton::clicked, this, &DPlayBackFunction::cancel);
}

void DPlayBackFunction::addTimeBar() {
	//replace the dock widget if exists
	this->ref_dockWidgetHandler->removeAndDeleteFloatDockWidget(this->m_timeBar);
	this->m_timeBar = new DPlayBackBar{};
	this->ref_dockWidgetHandler->addFloatDockWidget(this->m_timeBar, "Time Bar");
	//curret values
	this->m_timeBar->setSpeed(1000 / this->m_currentRate);
	this->m_timeBar->setCurrentTime(this->currentFrame() * this->ref_rec->getTimeStepSize() + this->ref_rec->getStartTime());
	//max min values
	this->m_timeBar->setStartEnd(this->ref_rec->getStartTime(), this->ref_rec->getEndTime(), this->ref_rec->getTimeStepSize());

	//connect from time bar
	this->m_timeBar->connectFunction(this);
	//connect from time line
	QObject::connect(this->ref_dockWidgetHandler, &DDockWidgetHandler::floatDockWidgetClosed, this, &DPlayBackFunction::timeBarClosed);
}

void DPlayBackFunction::delayPlotClicked() {
	//stop current playback
	this->pause();
	//remove the time bar
	this->ref_dockWidgetHandler->removeAndDeleteFloatDockWidget(this->m_timeBar);
	this->m_timeBar = nullptr;
	//options bar should handle by itself

	//make the function
	this->ref_functionHandler->addFunction(new DGenDelayPlotFunction(this->ref_rec));
}

void DPlayBackFunction::queuePlotClicked() {
	//stop current playback
	this->pause();
	//remove the time bar
	this->ref_dockWidgetHandler->removeAndDeleteFloatDockWidget(this->m_timeBar);
	this->m_timeBar = nullptr;
	//options bar should handle by itself

	//make the function
	this->ref_functionHandler->addFunction(new DGenQueueLengthFunction(this->ref_rec));
}

void DPlayBackFunction::timeBarPressed() {
	if (this->state() == QTimeLine::Running) {
		this->m_sliderRunLock = true;
		this->pause();
	}
	else 
		this->m_sliderRunLock = false;
}

void DPlayBackFunction::timeBarReleased() {
	if (this->m_sliderRunLock) {
		this->m_sliderRunLock = false;
		this->resume();
	}
}

void DPlayBackFunction::timeBarClosed(QWidget* widgetPtr) {
	if (widgetPtr == this->m_timeBar)
		this->m_timeBar = nullptr;
}

// --- --- --- --- --- Private Utils --- --- --- --- ---

void DPlayBackFunction::initGradients() {
	this->m_gradientList.clear();

	//Source :
	//http://www.kennethmoreland.com/color-advice/
	//"Diverging Color Maps for Scientific Visualization." Kenneth Moreland. In Proceedings of the 5th International Symposium on Visual Computing, December 2009. DOI 10.1007/978-3-642-10520-3_9.

	//red to green smooth
	QLinearGradient gradient_0{ 0, 0, 100, 0 };
	QColor tColor;
	tColor.setRgbF(0.758, 0.214, 0.233);
	gradient_0.setColorAt(1, tColor);
	tColor.setRgbF(0.865, 0.865, 0.865);
	gradient_0.setColorAt(0.5, tColor);
	tColor.setRgbF(0.085, 0.532, 0.201);
	gradient_0.setColorAt(0, tColor);
	this->m_gradientList.append(std::move(gradient_0));

	//Black Body
	QLinearGradient gradient_1{ 0, 0, 100, 0 };
	gradient_1.setColorAt(1, { 0,0,0 });
	gradient_1.setColorAt(0.61, { 178,34,34 });
	gradient_1.setColorAt(0.42, { 227,105,5 });
	gradient_1.setColorAt(0.16, { 238,210,20 });
	gradient_1.setColorAt(0, { 255,255,255 });
	this->m_gradientList.append(std::move(gradient_1));

	//Inferno
	QLinearGradient gradient_2{ 0, 0, 100, 0 };
	gradient_2.setColorAt(1, { 1,0,4 });
	gradient_2.setColorAt(0.86, { 48,7,84 });
	gradient_2.setColorAt(0.61, { 105,15,111 });
	gradient_2.setColorAt(0.57, { 158,40,100 });
	gradient_2.setColorAt(0.43, { 208,72,67 });
	gradient_2.setColorAt(0.29, { 239,125,21 });
	gradient_2.setColorAt(0.14, { 242,194,35 });
	gradient_2.setColorAt(0, { 245,255,163 });
	this->m_gradientList.append(std::move(gradient_2));
}
