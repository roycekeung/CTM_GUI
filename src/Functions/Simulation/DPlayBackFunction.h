#pragma once

#include <QTimeLine>
#include <QLinearGradient>

#include <I_Function.h>

class DPlayBackOptionsPanel;
class DPlayBackBar;
class QDockWidget;
struct RunRec;

namespace DISCO2_API {
class Rec_All;
}

class DPlayBackFunction : public QTimeLine, public I_Function {
	Q_OBJECT
private:

	size_t m_index;
	RunRec* ref_runRec = nullptr;
	DISCO2_API::Rec_All* ref_rec = nullptr;

	// --- --- --- --- --- UI Items --- --- --- --- ---

	DPlayBackOptionsPanel* m_optionsPanel = nullptr;
	DPlayBackBar* m_timeBar = nullptr;

	QList<QLinearGradient> m_gradientList;
	std::unordered_map<int, QColor> m_colorCache;
	int m_currentColorIndex = 0;

	int m_currentTextIndex = 0;

	double m_currentRate = 1;
	bool m_sliderRunLock = false;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DPlayBackFunction(size_t index);

	~DPlayBackFunction();

	void initFunctionHandler() override;

public slots:

	// --- --- --- --- --- Slots for Running --- --- --- --- ---

	void cancel();

	void inputTimeStepChanged(int value);

	void inputTimeChanged(double value);

	void inputSpeedChanged(int value);

	void currentTimeChanged(int value);

	void populateColorCache(int index);

	void textOptionChanged(int index);

	void pause();

	void startOrResume();

	void addOptionsPanel();

	void addTimeBar();

	void delayPlotClicked();

	void queuePlotClicked();

	void timeBarPressed();

	void timeBarReleased();

	void timeBarClosed(QWidget* widgetPtr);

private:

	// --- --- --- --- --- Private Utils --- --- --- --- ---

	void initGradients();

};
