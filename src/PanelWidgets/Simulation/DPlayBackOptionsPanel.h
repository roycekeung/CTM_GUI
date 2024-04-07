#pragma once

#include <QSize>
#include <QWidget>
#include "ui_DPlayBackOptionsPanel.h"

class QLinearGradient;

class DPlayBackOptionsPanel : public QWidget {
	Q_OBJECT
private:

	// --- --- --- --- --- UI Items --- --- --- --- ---

	Ui::DPlayBackOptionsPanel ui;

	const static QSize m_gradientSize;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DPlayBackOptionsPanel(QList<QLinearGradient>& gradients);

	~DPlayBackOptionsPanel();

	// --- --- --- --- --- Settings for Functions --- --- --- --- ---

	void setGradients(QList<QLinearGradient>& gradients);

	void setEditable(bool editable = true);

	// --- --- --- --- --- Getter --- --- --- --- ---
	
	QPushButton* getCloseButton();

	QComboBox* getStyleComboBox();

	QComboBox* getCellLabelComboBox();

};
