#pragma once

#include <vector>

#include <QWidget>
#include <QDialog>
#include "ui_DConflictMatrixPanel.h"

class DConflictMatrixFunction;
class DConflictMatrixItem;

class DConflictMatrixPanel : public QDialog {
	Q_OBJECT

private:

	// --- --- --- --- --- UI Items --- --- --- --- ---

	Ui::DConflictMatrixPanel ui;

	DConflictMatrixItem* m_jLItemDelegate = nullptr;
	DConflictMatrixItem* m_sGItemDelegate = nullptr;

	DConflictMatrixItem* m_jLLegendItemDelegate = nullptr;
	DConflictMatrixItem* m_sGLegendItemDelegate = nullptr;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DConflictMatrixPanel(bool hasSig);

	~DConflictMatrixPanel();

	// --- --- --- --- --- Getters --- --- --- --- ---

	QPushButton* getConflictButton();
	QPushButton* getPriorityButton();

	QModelIndex getCurrentSelectedJLCell();
	QModelIndex getCurrentSelectedSGCell();

	DConflictMatrixItem* getJLItemDelegate();
	DConflictMatrixItem* getSGItemDelegate();

	// --- --- --- --- --- Settings for Functions --- --- --- --- ---

	void setupConnections(DConflictMatrixFunction* function);

	void setJctLinkIds(std::vector<int>& ids);

	void setSigGpIds(std::vector<int>& ids);

	void updateTables();

public slots:

	// --- --- --- --- --- Slots --- --- --- --- ---

	void jLinkTableSelectionChanged();

	void sigGpTableSelectionChanged();

signals:

	// --- --- --- --- --- Signals --- --- --- --- ---

	void jLinkTableSelected();

	void sigGpTableSelected();

private:

	// --- --- --- --- --- Private Utils --- --- --- --- ---

	void setupTable(QTableWidget* table, std::vector<int>& ids);

	void setupLengends();

};
