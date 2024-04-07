#pragma once

#include <unordered_set>

// Qt lib
#include <QObject>

// DISCO2_GUI lib
#include "../I_Function.h"

// pre def
class DGraphicsScene;
class DCellConnectorItem;
class DHintClosePanel;

class DAddNewConnectorFunction : public QObject, public I_Function {
	Q_OBJECT
private:

	const static std::string hintText;

	DHintClosePanel* m_panel = nullptr;

	DCellConnectorItem* t_ConnectorItem = nullptr;

	int m_startId = -1;
	int m_fromType = -1;
	int m_endId = -1;
	int m_toType = -1;

	enum ClickStage { firstClick, createDrag };
	int m_clickStage = firstClick;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DAddNewConnectorFunction();

	~DAddNewConnectorFunction();

	void initFunctionHandler() override;

public slots:

	// --- --- --- --- --- Checking and creation Functions --- --- --- --- ---

	void checkAndCreateConnector();
	void cancel();

public:

	// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---

	bool keyPressEvent(QKeyEvent* keyEvent) override;
	bool mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
	bool mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

};
