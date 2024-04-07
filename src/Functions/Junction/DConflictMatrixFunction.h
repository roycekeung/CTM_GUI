#pragma once

#include <vector>
#include <list>

#include <QObject>

#include "Functions/I_Function.h"

class DConflictMatrixPanel;

class DConflictMatrixFunction : public QObject, public I_Function {
	Q_OBJECT

private:

	int m_jctId = -1;
	//cache such that there is a consistant order
	std::vector<int> m_jLinkIds;	
	std::vector<int> m_sigGpIds;

	std::vector<std::vector<bool>> m_defaultTable;
	std::list<std::tuple<int, int, bool>> m_overrideConflict;

	DConflictMatrixPanel* m_panel = nullptr;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DConflictMatrixFunction();

	~DConflictMatrixFunction();

	void initFunctionHandler() override;

public slots:

	// --- --- --- --- --- Slots for Panel --- --- --- --- ---

	void close();

	void clickedConflict();

	void clickedPriority();

	void clickedRevertAll();

	void hightlightJL();

	void hightlightSG();

private:

	// --- --- --- --- --- Private Utils --- --- --- --- ---

	std::string keyString(int id1, int id2);

	void calDefaultTable();

	void loadCustomSettings();

	void resetJLCellColors();

	void resetSGCellColors();

};
