#pragma once

#include <unordered_set>
#include <unordered_map>
//Qt stuff
#include <QWidget>
#include "ui_DSigGroupsPanel.h"
//DISCO stuff
namespace DISCO2_API {
class Jct_Node;
}
//DISCO GUI stuff
class DPseudoJctLinkItem;
class DSigGroupsFunction;

class DSigGroupsPanel : public QWidget {
	Q_OBJECT

private:

	// --- --- --- --- --- Hint Texts --- --- --- --- ---

	const static std::string hintText;

	// --- --- --- --- --- UI Items --- --- --- --- ---

	Ui::DSigGroupsPanel ui;

	// --- --- --- --- --- Ref to places --- --- --- --- ---

	const DISCO2_API::Jct_Node& ref_jctNode;
	std::unordered_map<int, DPseudoJctLinkItem*> ref_jctLinks;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DSigGroupsPanel(const DISCO2_API::Jct_Node& jctNode, std::unordered_map<int, DPseudoJctLinkItem*>&& jctLinks);
	
	~DSigGroupsPanel();

	// --- --- --- --- --- Settings for Function --- --- --- --- ---

	void setupConnections(DSigGroupsFunction* function);

	void failMessage(QString&& msg);

	void completedEditSigGp();

	// --- --- --- --- --- getters -- --- --- --- ---
	QTableWidget* getTableWdiget();

signals:

	// --- --- --- --- --- Signal for Function --- --- --- --- ---

	void editSigGpF(int sigGpId);

	void deleteSigGpF(int sigGpId);

public slots:

	// --- --- --- --- --- Slots --- --- --- --- ---

	void clickedEditSigGp();

	void clickedDeleteSigGp();

	void sigGpTableSelectionChanged();

public:

	// --- --- --- --- --- Utils --- --- --- --- ---

	void addSigGpToTable(int sigGpId);

	void updateSigGpTable(int sigGpId);

	void removeSigGpFromTable(int sigGpId);

};
