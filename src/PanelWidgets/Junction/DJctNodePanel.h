#pragma once

//Qt stuff
#include <QWidget>
#include <QString>
#include <QtWidgets/QLabel>
#include "ui_DJctNodePanel.h"
//DISCO stuff
namespace DISCO2_API {
class Jct_Node;
}
//DISCO GUI stuff
#include "DOkCancelButtons.h"
class DJctCreateFunction;
class DJctEditFunction;

class DJctNodePanel : public QWidget {
	Q_OBJECT

private:

	// --- --- --- --- --- UI Items --- --- --- --- ---

	Ui::DJctNodePanel ui;

	QLabel* hint_Label = nullptr;
	DOkCancelButtons* ok_Buttons = nullptr;
	QPushButton* jctModeButton = nullptr;

	// -- - -- - -- - -- - -- - Ref to places-- - -- - -- - -- - -- -

	const DISCO2_API::Jct_Node* ref_jctNode;

public:

	// --- --- --- --- --- Enum for Constructor --- --- --- --- ---

	enum class Type_JctPanel { view, create, edit };

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DJctNodePanel(Type_JctPanel type, const DISCO2_API::Jct_Node* jctNode = nullptr);

	~DJctNodePanel();


	// --- --- --- --- --- Settings for Functions --- --- --- --- ---

	void setEditable(bool editable = true);

	void enableConfirmButton();

	void setupCreateButtons(DJctCreateFunction* function);

	void setupEditButtons(DJctEditFunction* function);

	void failMessage(QString&& msg);

	// --- --- --- --- --- Getters for relaying input form values --- --- --- --- ---

	QString getNameInput();
	QString getDetailsInput();
	bool getIsSigInput();
	bool getIsBoxJctInput();
	
private:

	// --- --- --- --- --- Private Utils --- --- --- --- ---

	void insertHintLabel(Type_JctPanel type);

	void insertOkButtons();

	void insertJctModeButton();

	void showJctIdAndParam();

	void showJctDefaultParam();

};
