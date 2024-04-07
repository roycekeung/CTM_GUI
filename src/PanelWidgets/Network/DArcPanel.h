#pragma once

//Qt stuff
#include <QWidget>
#include <QString>
#include <QtWidgets/QLabel>
#include "ui_DArcPanel.h"
//DISCO stuff
namespace DISCO2_API {
class Arc;
class Jct_Link;
}
//DISCO GUI stuff
class DOkCancelButtons;
class DArcCreateFunction;
class DArcEditFunction;
class DJctLinkCreateFunction;
class DJctLinkEditFunction;
struct DefaultParamDataSet;

class DArcPanel : public QWidget {
	Q_OBJECT

private:

	// --- --- --- --- --- UI Items --- --- --- --- ---

	Ui::DArcPanel ui;

	QLabel* hint_Label = nullptr;
	DOkCancelButtons* ok_Buttons = nullptr;

	// --- --- --- --- --- Ref to places --- --- --- --- ---

	const DISCO2_API::Arc* ref_arc;
	DefaultParamDataSet* ref_dParam;
	const DISCO2_API::Jct_Link* ref_jctLink = nullptr;

public:

	// --- --- --- --- --- Enum for Constructor --- --- --- --- ---

	enum class Type_ArcPanel { view, create, edit, viewJL, createJL, editJL };

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DArcPanel(Type_ArcPanel type, const DISCO2_API::Arc* arc = nullptr, DefaultParamDataSet* dparam = nullptr);

	~DArcPanel();

	// --- --- --- --- --- Settings for Functions --- --- --- --- ---

	void setEditable(bool editable = true);

	void enableConfirmButton();

	void setupCreateButtons(DArcCreateFunction* function);

	void setupEditButtons(DArcEditFunction* function);

	void setupCreateJLButtons(DJctLinkCreateFunction* function);

	void setupEditJLButtons(DJctLinkEditFunction* function);

	void failMessage(QString&& msg);

	void setLengthValue(double length);

	// --- --- --- --- --- Getters for relaying input form values --- --- --- --- ---

	double getQInput();
	double getKInput();
	double getWInput();
	double getVfInput();
	double getLengthInput();
	int getNumOfLanesInput();
	QString getNameInput();
	QString getDetailsInput();

private:

	// --- --- --- --- --- Private Utils --- --- --- --- ---

	void insertHintLabel(Type_ArcPanel type);

	void insertOkButtons();

	void showArcIdAndParam();

	void showArcDefaultParam();

	void showJctLinkIdAndParam();

	void showJctLinkDefaultParam();

};
