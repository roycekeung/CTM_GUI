#pragma once

#include <unordered_set>

// Qt lib
#include <QObject>

// DISCO2_GUI lib
#include "../I_Function.h"

class DCellConnectorItem;
class DHintClosePanel;

class DRemoveConnectorFunction : public QObject, public I_Function {
	Q_OBJECT
private:

	const static std::string hintText;

	DHintClosePanel* m_panel = nullptr;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DRemoveConnectorFunction();

	~DRemoveConnectorFunction();

	void initFunctionHandler() override;

public slots:

	// --- --- --- --- --- Checking and creation Functions --- --- --- --- ---

	void checkAndRemoveConnector(DCellConnectorItem* item);
	void cancel();

public:

	// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---

	bool keyPressEvent(QKeyEvent* keyEvent) override;
	bool mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
	bool mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

private:

	// --- --- --- --- --- Private Util --- --- --- --- ---

	DCellConnectorItem* connectorAt(const QPointF& pt);

};

