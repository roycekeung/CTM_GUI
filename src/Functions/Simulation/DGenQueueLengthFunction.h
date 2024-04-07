#pragma once

#include <unordered_set>

#include <QObject>
#include <QColor>

#include <I_Function.h>

namespace DISCO2_API {
	class Rec_All;
}

class DGenQueueLengthPanel;

class DGenQueueLengthFunction : public QObject, public I_Function {
	Q_OBJECT
private:

	// --- --- --- --- --- Colors --- --- --- --- ---

	const static QColor refColor_tSelectStartColor;
	const static QColor refColor_tSelectEndColor;
	const static QColor refColor_selectedStartColor;
	const static QColor refColor_selectedEndColor;

	// --- --- --- --- --- Stuff --- --- --- --- ---

	const DISCO2_API::Rec_All* ref_rec;
	int m_startCellId = -1;
	int temp_cellId = -1;
	std::unordered_set<int> m_endCellIds;
	bool m_isSelectingStartCell = true;

	// --- --- --- --- --- UI Items --- --- --- --- ---

	DGenQueueLengthPanel* m_panel = nullptr;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DGenQueueLengthFunction(const DISCO2_API::Rec_All* rec);

	~DGenQueueLengthFunction();

	void initFunctionHandler() override;

public slots:

	// --- --- --- --- --- Slots for Panel --- --- --- --- ---

	void cancel();

	void reselectStartCell();

	void removeEndCell(int cellId);

	void removeAllEndCell();

	void confirmCellList();

public:

	// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---

	bool keyPressEvent(QKeyEvent* keyEvent) override;

	bool mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

	bool mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

};
