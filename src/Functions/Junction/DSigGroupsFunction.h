#pragma once

#include "I_Function.h"

#include <unordered_map>

#include <QObject>

class DSigGroupsPanel;

class DSigGroupsFunction : public QObject, public I_Function {
	Q_OBJECT

private:

	int m_jctId = -1;

	DSigGroupsPanel* m_panelWidet = nullptr;

	bool m_selectMode = false;
	int t_currSigGp = -1;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DSigGroupsFunction();

	~DSigGroupsFunction();

	void initFunctionHandler() override;

public slots:

	// --- --- --- --- --- Checking and creation Functions --- --- --- --- ---

	void addNewSigGp();

	void editSigGp(int sigGpId);

	void deleteSigGp(int sigGpId);

	void cancel();

public:

	// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---

	bool keyPressEvent(QKeyEvent* keyEvent) override;

	bool mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

};
