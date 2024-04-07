#pragma once

// std lib
#include<string>  
// Qt lib
#include <QCloseEvent>
#include <QObject>
// DISCO_GUI lib
#include "../Functions/I_Function.h"

class DSigDemandFileDialogue;

class DSigDemandFileFunction : public QObject, public I_Function{
	Q_OBJECT
public:
	// --- --- --- --- --- Enum for Constructor --- --- --- --- ---
	enum Type_File { Signal, Demand };
	enum ColoumnHead { ID, FNm, Nm, Dtl };

private:
	// --- --- --- --- --- storing of UI and Enum type --- --- --- --- ---
	Type_File type;

	// a widget that really holds the UI
	DSigDemandFileDialogue* DSigDemandFile;

	// --- --- --- Default set up functions --- --- --- --- ---
	void setDefualtSignalTable();
	void setDefualtDemandTable();
	void connectAllButtons();

	// --- --- --- --- --- Inner Functions --- --- --- --- ---
	void addOneDefualtRow();
	void addOneRow(int setID, QString FileBaseName, std::string Name, std::string Details);
	void saveFile();
	void LoadFile();
	void RemoveFile();
	void CopyFile();

	void SigSaveintoCore(int setID, QString absoluteFilePath_FileName, QString Name, QString Details);
	void SigSaveintoFileAndCore(int setID, QString absoluteFilePath_FileName, QString Name, QString Details);
	void DemSaveintoCore(int setID, QString absoluteFilePath_FileName, QString Name, QString Details);
	void DemSaveintoFileAndCore(int setID, QString absoluteFilePath_FileName, QString Name, QString Details);

	int getIDhaventused();

	// --- --- --- --- --- ---  Error Message creation --- --- --- --- --- ---  
	void CreateErrorBoxForRow(int ID, std::string ErrorMessage);
	void CreateErrorBoxForCatchE(std::exception& e);

public:
	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---
	DSigDemandFileFunction(Type_File type);
	~DSigDemandFileFunction();

	// --- --- --- --- --- override initFunctionHandler from I_Function --- --- --- --- ---
	void initFunctionHandler() override;

public slots:

	// --- --- --- --- --- Slots for Panel --- --- --- --- ---

	void dialogFinished();

	void saveAllFilesfunc();

	void setCurrentSelectedSetFile();
};


