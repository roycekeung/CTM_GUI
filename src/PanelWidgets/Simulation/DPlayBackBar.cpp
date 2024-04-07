#include "DPlayBackBar.h"

#include "Functions/Simulation/DPlayBackFunction.h"

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DPlayBackBar::DPlayBackBar() : QWidget(nullptr) {
	this->ui.setupUi(this);
	this->ui.speed_spinBox->setValue(1);
}

DPlayBackBar::~DPlayBackBar() {
}

// --- --- --- --- --- Setters --- --- --- --- ---

void DPlayBackBar::connectFunction(DPlayBackFunction* function) {
	QObject::connect(this->ui.start_pushButton, &QPushButton::clicked, function, &DPlayBackFunction::startOrResume);
	QObject::connect(this->ui.pause_pushButton, &QPushButton::clicked, function, &DPlayBackFunction::pause);
	QObject::connect(this->ui.speed_spinBox, QOverload<int>::of(&QSpinBox::valueChanged), function, &DPlayBackFunction::inputSpeedChanged);
	QObject::connect(this->ui.horizontalSlider, &QSlider::valueChanged, function, &DPlayBackFunction::inputTimeStepChanged);
	QObject::connect(this->ui.time_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), function, &DPlayBackFunction::inputTimeChanged);
	QObject::connect(this->ui.horizontalSlider, &QSlider::sliderPressed, function, &DPlayBackFunction::timeBarPressed);
	QObject::connect(this->ui.horizontalSlider, &QSlider::sliderReleased, function, &DPlayBackFunction::timeBarReleased);
}

void DPlayBackBar::setStartEnd(int64_t start, int64_t end, int tSize) {
	this->m_startTime = start;
	this->m_endTime = end;
	this->m_timeStepSize = tSize;

	this->ui.horizontalSlider->setMinimum(0);
	this->ui.horizontalSlider->setMaximum((end - start) / tSize);

	this->ui.time_doubleSpinBox->setMinimum(start);
	this->ui.time_doubleSpinBox->setMaximum(end);
}

void DPlayBackBar::setCurrentTime(int64_t time) {
	if (time > this->m_startTime) {
		this->ui.horizontalSlider->setValue((time - this->m_startTime) / this->m_timeStepSize);
		this->ui.time_doubleSpinBox->setValue((1.0/1000)*time);
	}
}

void DPlayBackBar::setSpeed(int value) {
	this->ui.speed_spinBox->setValue(value);
}
