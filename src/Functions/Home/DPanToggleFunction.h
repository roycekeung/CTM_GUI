#pragma once

// Qt lib
#include <QObject>
#include <QGraphicsView>

// DISCO2_GUI lib
#include "../Functions/I_Function.h"

// pre def
class QPushButton;
class QGraphicsView;

class DPanToggleFunction : public QObject, public I_Function {
	Q_OBJECT
private:
	QPushButton* m_ToggleBtn = nullptr;

	QGraphicsView::DragMode prev_dragMode;

public:
	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---
	DPanToggleFunction(QPushButton* ToggleButton);
	~DPanToggleFunction();

	// --- --- --- --- --- override initFunctionHandler from I_Function --- --- --- --- ---
	void initFunctionHandler() override;

public slots:

	// --- --- --- --- --- Close Functions --- --- --- --- ---
	void cancel(bool checked);

};
