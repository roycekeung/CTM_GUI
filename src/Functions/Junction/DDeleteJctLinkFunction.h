#pragma once

#include <QObject>
class QPushButton;

#include "Functions/I_Function.h"
class DHintClosePanel;

class DDeleteJctLinkFunction : public QObject, public I_Function {
	Q_OBJECT

private:

	const static std::string hintText;

	QPushButton* m_deleteButton;

	DHintClosePanel* m_panel = nullptr;

	int m_jctId = -1;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DDeleteJctLinkFunction(QPushButton* deleteJctLinkButton);

	~DDeleteJctLinkFunction();

	void initFunctionHandler() override;

public slots:

	// --- --- --- --- --- Slot for panel --- --- --- --- ---

	void cancel();

public:

	// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---

	bool keyPressEvent(QKeyEvent* keyEvent) override;

	bool mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

	bool mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

};
