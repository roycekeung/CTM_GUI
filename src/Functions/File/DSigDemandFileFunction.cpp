#include "DSigDemandFileFunction.h"
#include "ui_DSigDemandFileDialogue.h"

// DISCO_GUI lib
#include "PanelWidgets/FileTab/DSigDemandFileDialogue.h"
#include "DMainWindow.h"
#include "DTabToolBar.h"
// Qt lib
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <QObject>
#include <QSpinBox>


// --- --- --- --- --- Constructor Destructor --- --- --- --- ---
DSigDemandFileFunction::DSigDemandFileFunction(Type_File type) : QObject(), type(type){
}

DSigDemandFileFunction::~DSigDemandFileFunction() {
	this->ref_tabToolBar->setEnabled(true);
}

// --- --- --- --- --- override initFunctionHandler from I_Function --- --- --- --- ---
void DSigDemandFileFunction::initFunctionHandler() {
	// New UI Widget, prepared for use
	this->DSigDemandFile = new DSigDemandFileDialogue();

	/// connect the save and load button within this DSigDemandFile dialogue to the corresponding function
	this->connectAllButtons();
	this->ref_tabToolBar->setEnabled(false);

	// determine type
	switch (this->type) {
	case Type_File::Signal: {
		// basic UI set up
		this->DSigDemandFile->setWindowTitle("Signal Plan File Dialog");
		this->DSigDemandFile->getUI().SaveLoadFileLabel->setText("Signal Plan Lists");
		setDefualtSignalTable();
		break;
	}
	case Type_File::Demand: {
		// basic UI set up
		this->DSigDemandFile->setWindowTitle("Demand Profile Dialog");
		this->DSigDemandFile->getUI().SaveLoadFileLabel->setText("Demand Profile Lists");
		setDefualtDemandTable();
		break;
	}
	} // end of switch type

	// basic UI set up
	this->DSigDemandFile->resize(510, 500);

	/// Set row highlight when selected
	this->DSigDemandFile->getUI().TableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	// turn off the row header of qtablewidget 
	this->DSigDemandFile->getUI().TableWidget->verticalHeader()->hide();
	// shows tool tips
	this->DSigDemandFile->setAttribute(Qt::WA_AlwaysShowToolTips, true);
	//QHeaderView will automatically resize the section to fill the available space horizontally, fixed vertically. 
	this->DSigDemandFile->getUI().TableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	this->DSigDemandFile->getUI().TableWidget->verticalHeader()->setMinimumSectionSize(0);
	this->DSigDemandFile->getUI().TableWidget->verticalHeader()->setOffset(0);

	// turn off the built-in help question mark button
	this->DSigDemandFile->setWindowFlags(this->DSigDemandFile->windowFlags() & ~Qt::WindowContextHelpButtonHint);

	// run dialog
	this->DSigDemandFile->exec();

}

// --- --- --- Default set up functions --- --- --- --- ---
void DSigDemandFileFunction::setDefualtSignalTable() {
	/// get total number of sigset it have in core
	int RowCount = (int)this->ref_scn->get()->getSignal().getSigSetIds().size();

	// defualt in 1 row
	// if totalRowCount > 0 ; means core have the the setid already, so just list out the current saved inf unless press 
	// New button to create new setid then it would 
	if (this->DSigDemandFile->getUI().TableWidget->rowCount() == 0 && RowCount == 0) {
		addOneDefualtRow();
	}

	// inserting the inner value in core into the table widget
	if (RowCount != 0) {
		// set the table row count match with the core row count
		this->DSigDemandFile->getUI().TableWidget->setRowCount(RowCount);

		int row = 0;
		// load the sigset inf from core to table 
		for (auto SigSetID : this->ref_scn->get()->getSignal().getSigSetIds()) {
			// get current Sig_VisInfo in core
			DISCO2_API::VisInfo Sig_VisInfo = this->ref_scn->get()->getSignal().getSigSet(SigSetID).getVisInfo();

			// getting QTableWidgetItem item for inserting value into the table one cell by one cell
			//QTableWidgetItem* IDCell = this->DSigDemandFile->getUI().TableWidget->item(row, ColoumnHead::ID);
			QTableWidgetItem* FileNameCell = this->DSigDemandFile->getUI().TableWidget->item(row, ColoumnHead::FNm);
			QTableWidgetItem* NameCell = this->DSigDemandFile->getUI().TableWidget->item(row, ColoumnHead::Nm);
			QTableWidgetItem* DetailsCell = this->DSigDemandFile->getUI().TableWidget->item(row, ColoumnHead::Dtl);

			// set IDCell
			QSpinBox* IDCell = new QSpinBox(this->DSigDemandFile->getUI().TableWidget);
			IDCell->setValue(SigSetID);
			IDCell->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
			IDCell->setButtonSymbols(QAbstractSpinBox::NoButtons);
			this->DSigDemandFile->getUI().TableWidget->setCellWidget(row, ColoumnHead::ID, IDCell);
			IDCell->setReadOnly(true);  // not editable but can select
			IDCell->setFrame(false);  // no frame

			// set FileNameCell
			FileNameCell = new QTableWidgetItem;
			this->DSigDemandFile->getUI().TableWidget->setItem(row, ColoumnHead::FNm, FileNameCell);
			QFileInfo FileNameInfo(Sig_VisInfo.findSecret("FileName").c_str());
			FileNameCell->setText(FileNameInfo.baseName());
			FileNameCell->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
			//FileNameCell->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
			//FileNameCell->setFlags(FileNameCell->flags() & ~Qt::ItemIsEditable);  // not editable

			// set NameCell
			NameCell = new QTableWidgetItem;
			this->DSigDemandFile->getUI().TableWidget->setItem(row, ColoumnHead::Nm, NameCell);
			NameCell->setText(Sig_VisInfo.getName().c_str());
			NameCell->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
			//NameCell->setFlags(FileNameCell->flags() & ~Qt::ItemIsEditable);    // not editable

			// set DetailsCell
			DetailsCell = new QTableWidgetItem;
			this->DSigDemandFile->getUI().TableWidget->setItem(row, ColoumnHead::Dtl, DetailsCell);
			DetailsCell->setText(Sig_VisInfo.getDetails().c_str());
			DetailsCell->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
			//NameCell->setFlags(FileNameCell->flags() & ~Qt::ItemIsEditable);    // not editable

			row++;
		}// end of sigset looping
	}// end of row looping
}

void DSigDemandFileFunction::setDefualtDemandTable() {
	/// get total number of sigset it have in core
	int RowCount = (int)this->ref_scn->get()->getDemand().getDemandSetIds().size();

	// defualt in 1 row
	// if totalRowCount > 0 ; means core have the the setid already, so just list out the current saved inf unless press 
	// New button to create new setid then it would 
	if (this->DSigDemandFile->getUI().TableWidget->rowCount() == 0 && RowCount == 0) {
		addOneDefualtRow();
	}

	// inserting the inner value in core into the table widget
	if (RowCount != 0) {
		// set the table row count match with the core row count
		this->DSigDemandFile->getUI().TableWidget->setRowCount(RowCount);

		int row = 0;
		// load the sigset inf from core to table 
		for (auto DemSetID : this->ref_scn->get()->getDemand().getDemandSetIds()) {
			// get current Sig_VisInfo in core
			DISCO2_API::VisInfo Dem_VisInfo = this->ref_scn->get()->getDemand().getDemandSet(DemSetID).getVisInfo();

			// getting QTableWidgetItem item for inserting value into the table one cell by one cell
			//QTableWidgetItem* IDCell = this->DSigDemandFile->getUI().TableWidget->item(row, ColoumnHead::ID);
			QTableWidgetItem* FileNameCell = this->DSigDemandFile->getUI().TableWidget->item(row, ColoumnHead::FNm);
			QTableWidgetItem* NameCell = this->DSigDemandFile->getUI().TableWidget->item(row, ColoumnHead::Nm);
			QTableWidgetItem* DetailsCell = this->DSigDemandFile->getUI().TableWidget->item(row, ColoumnHead::Dtl);

			// set IDCell
			QSpinBox* IDCell = new QSpinBox(this->DSigDemandFile->getUI().TableWidget);
			IDCell->setValue(DemSetID);
			IDCell->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
			IDCell->setButtonSymbols(QAbstractSpinBox::NoButtons);
			this->DSigDemandFile->getUI().TableWidget->setCellWidget(row, ColoumnHead::ID, IDCell);
			IDCell->setReadOnly(true);  // not editable but can select
			IDCell->setFrame(false);  // no frame

			// set FileNameCell
			FileNameCell = new QTableWidgetItem;
			this->DSigDemandFile->getUI().TableWidget->setItem(row, ColoumnHead::FNm, FileNameCell);
			QFileInfo FileNameInfo(Dem_VisInfo.findSecret("FileName").c_str());
			FileNameCell->setText(FileNameInfo.baseName());
			FileNameCell->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
			//FileNameCell->setFlags(FileNameCell->flags() & ~Qt::ItemIsEditable);  // not editable

			// set NameCell
			NameCell = new QTableWidgetItem;
			this->DSigDemandFile->getUI().TableWidget->setItem(row, ColoumnHead::Nm, NameCell);
			NameCell->setText(Dem_VisInfo.getName().c_str());
			NameCell->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
			//NameCell->setFlags(FileNameCell->flags() & ~Qt::ItemIsEditable);    // not editable

			// set DetailsCell
			DetailsCell = new QTableWidgetItem;
			this->DSigDemandFile->getUI().TableWidget->setItem(row, ColoumnHead::Dtl, DetailsCell);
			DetailsCell->setText(Dem_VisInfo.getDetails().c_str());
			DetailsCell->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
			//NameCell->setFlags(FileNameCell->flags() & ~Qt::ItemIsEditable);    // not editable

			row++;
		}// end of demset looping
	}// end of row looping
}

void DSigDemandFileFunction::connectAllButtons() {
	// connect the save and load button within this DSigDemandFile dialogue to the corresponding function
	QObject::connect(this->DSigDemandFile->getUI().NewButton, &QPushButton::clicked, this, &DSigDemandFileFunction::addOneDefualtRow);
	QObject::connect(this->DSigDemandFile->getUI().SaveButton, &QPushButton::clicked, this, &DSigDemandFileFunction::saveFile);
	QObject::connect(this->DSigDemandFile->getUI().LoadButton, &QPushButton::clicked, this, &DSigDemandFileFunction::LoadFile);
	QObject::connect(this->DSigDemandFile->getUI().RemoveButton, &QPushButton::clicked, this, &DSigDemandFileFunction::RemoveFile);
	QObject::connect(this->DSigDemandFile->getUI().CopyButton, &QPushButton::clicked, this, &DSigDemandFileFunction::CopyFile);
	QObject::connect(this->DSigDemandFile->getUI().close_pushButton, &QPushButton::clicked, this->DSigDemandFile, &DSigDemandFileDialogue::accept);

	QObject::connect(this->DSigDemandFile, &QDialog::finished, this, &DSigDemandFileFunction::dialogFinished);
}

// --- --- --- --- --- Inner Functions --- --- --- --- ---
void DSigDemandFileFunction::addOneDefualtRow() {
	// TableRowCount in UI
	int TableRowCount = this->DSigDemandFile->getUI().TableWidget->rowCount();

	// add one more empty row
	this->DSigDemandFile->getUI().TableWidget->setRowCount(std::max(1, TableRowCount +1));

	// defualt IDCell initial set up
	QSpinBox* IDCell = new QSpinBox(this->DSigDemandFile->getUI().TableWidget);
	IDCell->setValue(getIDhaventused());
	IDCell->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	IDCell->setButtonSymbols(QAbstractSpinBox::NoButtons);
	this->DSigDemandFile->getUI().TableWidget->setCellWidget(std::max(0, TableRowCount), ColoumnHead::ID, IDCell);
	IDCell->setToolTip(tr("New"));
	IDCell->setReadOnly(true);  // not editable but can select
	IDCell->setFrame(false);  // no frame

	// defualt FileNameCell initial set up
	QTableWidgetItem* FileNameCell = this->DSigDemandFile->getUI().TableWidget->item(0, ColoumnHead::FNm);
	// dont have  FileNameCell , it could only be added externally
	FileNameCell = new QTableWidgetItem;
	this->DSigDemandFile->getUI().TableWidget->setItem(std::max(0, TableRowCount), ColoumnHead::FNm, FileNameCell);
	FileNameCell->setText(QString::fromUtf8("(New)") );
	FileNameCell->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	FileNameCell->setFlags(FileNameCell->flags() | Qt::ItemIsEditable);    // editable
	FileNameCell->setToolTip(tr("New"));

	// defualt NameCell initial set up
	QTableWidgetItem* NameCell = this->DSigDemandFile->getUI().TableWidget->item(0, ColoumnHead::Nm);
	NameCell = new QTableWidgetItem;
	this->DSigDemandFile->getUI().TableWidget->setItem(std::max(0, TableRowCount), ColoumnHead::Nm, NameCell);
	NameCell->setText(QString::fromUtf8("(New)"));
	NameCell->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	NameCell->setFlags(NameCell->flags() | Qt::ItemIsEditable);    // editable
	NameCell->setToolTip(tr("New"));

	// defualt DetailsCell initial set up
	QTableWidgetItem* DetailsCell = this->DSigDemandFile->getUI().TableWidget->item(0, ColoumnHead::Nm);
	DetailsCell = new QTableWidgetItem;
	this->DSigDemandFile->getUI().TableWidget->setItem(std::max(0, TableRowCount), ColoumnHead::Dtl, DetailsCell);
	DetailsCell->setText(QString::fromUtf8("(New)"));
	DetailsCell->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	DetailsCell->setFlags(DetailsCell->flags() | Qt::ItemIsEditable);    // editable
	DetailsCell->setToolTip(tr("New"));

}

void DSigDemandFileFunction::addOneRow(int setID, QString FileBaseName, std::string Name, std::string Details) {
	// TableRowCount in UI
	int TableRowCount = this->DSigDemandFile->getUI().TableWidget->rowCount();

	// add one more empty row
	this->DSigDemandFile->getUI().TableWidget->setRowCount(std::max(1, TableRowCount + 1));

	// defualt IDCell initial set up
	QSpinBox* IDCell = new QSpinBox(this->DSigDemandFile->getUI().TableWidget);
	IDCell->setValue(setID);
	IDCell->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	IDCell->setButtonSymbols(QAbstractSpinBox::NoButtons);
	this->DSigDemandFile->getUI().TableWidget->setCellWidget(std::max(0, TableRowCount), ColoumnHead::ID, IDCell);
	IDCell->setToolTip(tr("loaded"));
	IDCell->setReadOnly(true);  // not editable but can select
	IDCell->setFrame(false);  // no frame

	// defualt FileNameCell initial set up
	QTableWidgetItem* FileBaseNameCell = this->DSigDemandFile->getUI().TableWidget->item(0, ColoumnHead::FNm);
	// dont have  FileNameCell , it could only be added externally
	FileBaseNameCell = new QTableWidgetItem;
	this->DSigDemandFile->getUI().TableWidget->setItem(std::max(0, TableRowCount), ColoumnHead::FNm, FileBaseNameCell);
	FileBaseNameCell->setText(FileBaseName);
	FileBaseNameCell->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	FileBaseNameCell->setFlags(FileBaseNameCell->flags() | Qt::ItemIsEditable);    // editable
	FileBaseNameCell->setToolTip(tr("loaded"));

	// defualt NameCell initial set up
	QTableWidgetItem* NameCell = this->DSigDemandFile->getUI().TableWidget->item(0, ColoumnHead::Nm);
	NameCell = new QTableWidgetItem;
	this->DSigDemandFile->getUI().TableWidget->setItem(std::max(0, TableRowCount), ColoumnHead::Nm, NameCell);
	NameCell->setText(QString::fromStdString(Name));
	NameCell->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	NameCell->setFlags(NameCell->flags() | Qt::ItemIsEditable);    // editable
	NameCell->setToolTip(tr("loaded"));

	// defualt DetailsCell initial set up
	QTableWidgetItem* DetailsCell = this->DSigDemandFile->getUI().TableWidget->item(0, ColoumnHead::Nm);
	DetailsCell = new QTableWidgetItem;
	this->DSigDemandFile->getUI().TableWidget->setItem(std::max(0, TableRowCount), ColoumnHead::Dtl, DetailsCell);
	DetailsCell->setText(QString::fromStdString(Details));
	DetailsCell->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	DetailsCell->setFlags(DetailsCell->flags() | Qt::ItemIsEditable);    // editable
	DetailsCell->setToolTip(tr("loaded"));

}

void DSigDemandFileFunction::saveFile() {
	// get the selected row
	int select_rowidx = this->DSigDemandFile->getUI().TableWidget->selectionModel()->currentIndex().row();

	// getting selected QTableWidgetItem items
	QSpinBox* IDCell = static_cast<QSpinBox*>(this->DSigDemandFile->getUI().TableWidget->cellWidget(select_rowidx, ColoumnHead::ID));
	QTableWidgetItem* FileNameCell = this->DSigDemandFile->getUI().TableWidget->item(select_rowidx, ColoumnHead::FNm);
	QTableWidgetItem* NameCell = this->DSigDemandFile->getUI().TableWidget->item(select_rowidx, ColoumnHead::Nm);
	QTableWidgetItem* DetailsCell = this->DSigDemandFile->getUI().TableWidget->item(select_rowidx, ColoumnHead::Dtl);

	// getting selected QTableWidgetItem variables
	int setID = IDCell->value();
	QString fileName = FileNameCell->data(Qt::DisplayRole).toString();
	QString absoluteFilePath_FileName;
	QString Name = NameCell->data(Qt::DisplayRole).toString();
	QString Details = DetailsCell->data(Qt::DisplayRole).toString();

	if (this->type == Type_File::Signal) {
		// open the save file dialogue
		
		//QString QFileDialog::getSaveFileName(QWidget *parent = nullptr, const QString &caption = QString(), const QString &dir = QString(), const QString &filter = QString(), QString *selectedFilter = nullptr, QFileDialog::Options options = Options())
		absoluteFilePath_FileName = QFileDialog::getSaveFileName(this->DSigDemandFile, QObject::tr("Save Signal Plan"), fileName, QObject::tr("XML files (*.xml)"));

		// save the row wise inf into the core
		SigSaveintoFileAndCore(setID, absoluteFilePath_FileName, Name, Details);
	}
	else if (this->type == Type_File::Demand) {
		// open the save file dialogue
		absoluteFilePath_FileName = QFileDialog::getSaveFileName(this->DSigDemandFile, QObject::tr("Save Demand Profile"), fileName, QObject::tr("XML files (*.xml)"));

		// save the row wise inf into the core
		DemSaveintoFileAndCore(setID, absoluteFilePath_FileName, Name, Details);
	}


	// set IDCell
	IDCell->setEnabled(false);  // not editable
	IDCell->setToolTip(tr("saved"));

	// set FileNameCell
	QFileInfo FileNameInfo(absoluteFilePath_FileName);
	FileNameCell->setText(FileNameInfo.baseName());
	//FileNameCell->setFlags(FileNameCell->flags() & ~Qt::ItemIsEditable);  // not editable
	FileNameCell->setToolTip(tr("saved"));

	// set NameCell
	//NameCell->setFlags(NameCell->flags() & ~Qt::ItemIsEditable);    // not editable
	NameCell->setToolTip(tr("saved"));

	// set DetailsCell
	//DetailsCell->setFlags(DetailsCell->flags() & ~Qt::ItemIsEditable);    // not editable
	DetailsCell->setToolTip(tr("saved"));
}


void DSigDemandFileFunction::LoadFile() {
	int tmp_setID ;
	QString absoluteFilePath_FileName;
	if (this->type == Type_File::Signal) {
		// open the save file dialogue
		absoluteFilePath_FileName = QFileDialog::getOpenFileName(this->DSigDemandFile, QObject::tr("Save Signal Plan"), QObject::tr("Load Signal Plan"), QObject::tr("XML files (*.xml)"));
		// specific file info for this set file, will be saving internally for avoiding same filename overwriting 
		QFileInfo FileNameInfo(absoluteFilePath_FileName);

		if (!absoluteFilePath_FileName.isEmpty()) {
			// load file in, got the file name
			try {
				// load from the core
				tmp_setID = this->ref_scn->get()->loadSignalSet(absoluteFilePath_FileName.toUtf8().constData());

				DISCO2_API::VisInfo Sig_VisInfo = this->ref_scn->get()->getSignal().getSigSet(tmp_setID).getVisInfo();

				addOneRow(tmp_setID, FileNameInfo.baseName(), Sig_VisInfo.getName().c_str(), Sig_VisInfo.getDetails().c_str());

			}
			catch (std::exception& e) {
				CreateErrorBoxForCatchE(e);
				return;
			}
		}
		// else, didnt even save any file in, u cancel the action

	}
	else if (this->type == Type_File::Demand) {
		// load file in, got the file name
		// open the save file dialogue
		absoluteFilePath_FileName = QFileDialog::getOpenFileName(this->DSigDemandFile, QObject::tr("Save Demand Profile"), QObject::tr("Load Demand Profile"), QObject::tr("XML files (*.xml)"));
		// specific file info for this set file, will be saving internally for avoiding same filename overwriting 
		QFileInfo FileNameInfo(absoluteFilePath_FileName);

		if (!absoluteFilePath_FileName.isEmpty()) {
			// load file in, got the file name
			try {
				// load from the core
				tmp_setID = this->ref_scn->get()->loadDemandSet(absoluteFilePath_FileName.toUtf8().constData());

				DISCO2_API::VisInfo Dem_VisInfo = this->ref_scn->get()->getDemand().getDemandSet(tmp_setID).getVisInfo();

				addOneRow(tmp_setID, FileNameInfo.baseName(), Dem_VisInfo.getName().c_str(), Dem_VisInfo.getDetails().c_str());
			}
			catch (std::exception& e) {
				CreateErrorBoxForCatchE(e);
				return;
			}
		}
		// else, didnt even save any file in, u cancel the action

	}// end of switch type

}

void DSigDemandFileFunction::RemoveFile() {
	// delete row file and also delete that of record according to the corresponding setID

	// empty table activate this
	if (this->DSigDemandFile->getUI().TableWidget->rowCount() == 0) {
		return;
	}

	// get the selected row
	int total_TableRowCount = this->DSigDemandFile->getUI().TableWidget->rowCount();
	int rowID = 0;
	for (int loopCount = 0; rowID < total_TableRowCount; loopCount++) {

		if (this->DSigDemandFile->getUI().TableWidget->selectionModel()->isRowSelected(rowID)) {
			// getting selected QTableWidgetItem items
			QSpinBox* IDCell = static_cast<QSpinBox*>(this->DSigDemandFile->getUI().TableWidget->cellWidget(rowID, ColoumnHead::ID));
			// getting selected QTableWidgetItem variables
			int setID = IDCell->value();

			// remove from the UI table
			this->DSigDemandFile->getUI().TableWidget->removeRow(rowID);

			if (this->type == Type_File::Signal) {
				// remove that sigset from core
				this->ref_scn->get()->getSigEditor().sigSet_delete(setID);
			}
			else if (this->type == Type_File::Demand) {
				// remove that dmdset from core
				this->ref_scn->get()->getDmdEditor().dmdSet_delete(setID);
			}

			// stay on same row coz this row is deleted, the following would shift upwards
		}
		else{
			// move to next row
			rowID++;
		}

	
	}

}

void DSigDemandFileFunction::CopyFile() {
	// Clone row file and also Clone that of record according to the previous selected setID, then generated new SetID

	// empty table activate this
	if (this->DSigDemandFile->getUI().TableWidget->rowCount() == 0) {
		return;
	}

	int New_setID;

	// get the selected row
	int select_rowidx = this->DSigDemandFile->getUI().TableWidget->selectionModel()->currentIndex().row();

	// getting selected QTableWidgetItem items
	QSpinBox* IDCell = static_cast<QSpinBox*>(this->DSigDemandFile->getUI().TableWidget->cellWidget(select_rowidx, ColoumnHead::ID));
	QTableWidgetItem* FileNameCell = this->DSigDemandFile->getUI().TableWidget->item(select_rowidx, ColoumnHead::FNm);
	QTableWidgetItem* NameCell = this->DSigDemandFile->getUI().TableWidget->item(select_rowidx, ColoumnHead::Nm);
	QTableWidgetItem* DetailsCell = this->DSigDemandFile->getUI().TableWidget->item(select_rowidx, ColoumnHead::Dtl);

	// getting selected QTableWidgetItem variables
	int setID = IDCell->value();
	QString fileName = FileNameCell->data(Qt::DisplayRole).toString();
	QString Name = NameCell->data(Qt::DisplayRole).toString();
	QString Details = DetailsCell->data(Qt::DisplayRole).toString();

	if (this->type == Type_File::Signal) {
		// remove that sigset from core
		New_setID = this->ref_scn->get()->getSigEditor().sigSet_clone(setID);
	}
	else if (this->type == Type_File::Demand) {
		// remove that dmdset from core
		New_setID =  this->ref_scn->get()->getDmdEditor().dmdSet_clone(setID);
	}

	addOneRow(New_setID, fileName, Name.toStdString(), Details.toStdString());

}

void DSigDemandFileFunction::SigSaveintoCore(int setID, QString absoluteFilePath_FileName, QString Name, QString Details) {

	if (!absoluteFilePath_FileName.isEmpty()) {
		// save file in, got the file name
		try {
			// save into the core
			auto& sigSet = this->ref_scn->get()->getSignal().getSigSet(setID);
			auto visInfo = sigSet.getVisInfo();
			visInfo.setName(Name.toStdString());
			visInfo.setDetails(Details.toStdString());
			visInfo.removeSecret("FileName");
			visInfo.addSecret("FileName", absoluteFilePath_FileName.toStdString());
			this->ref_scn->get()->getSigEditor().sigSet_setVisInfo(setID, std::move(visInfo));
		}
		catch (std::exception& e) {
			CreateErrorBoxForCatchE(e);
		}
	}
	else {
		// any row that newly created but without the naming would pop out missing file name error
		CreateErrorBoxForRow(setID, std::string("file name is empty, saving action is failed"));
	}
}

void DSigDemandFileFunction::SigSaveintoFileAndCore(int setID, QString absoluteFilePath_FileName, QString Name, QString Details) {

	if (!absoluteFilePath_FileName.isEmpty()) {
		// save file in, got the file name
		try {
			// save into the core
			auto& sigSet = this->ref_scn->get()->getSignal().getSigSet(setID);
			auto visInfo = sigSet.getVisInfo();
			visInfo.setName(Name.toStdString());
			visInfo.setDetails(Details.toStdString());
			visInfo.removeSecret("FileName");
			// overwrite into VisInfo to the editor in core; here Secrets is empty
			this->ref_scn->get()->getSigEditor().sigSet_setVisInfo(setID, DISCO2_API::VisInfo{ visInfo });

			// save signal into file
			this->ref_scn->get()->saveSignalSet(setID, absoluteFilePath_FileName.toUtf8().constData());

			// save the absolute file path and the file name into the secrets
			visInfo.addSecret("FileName", absoluteFilePath_FileName.toStdString());
			this->ref_scn->get()->getSigEditor().sigSet_setVisInfo(setID, std::move(visInfo));
		}
		catch (std::exception& e) {
			CreateErrorBoxForCatchE(e);
		}
	}
	// allows cancel of the file saving dialogue
}

void DSigDemandFileFunction::DemSaveintoCore(int setID, QString absoluteFilePath_FileName, QString Name, QString Details) {

	if (!absoluteFilePath_FileName.isEmpty()) {
		// save file in, got the file name
		try {
			// save into the core
			auto& dmdSet = this->ref_scn->get()->getDemand().getDemandSet(setID);
			auto visInfo = dmdSet.getVisInfo();
			visInfo.setName(Name.toStdString());
			visInfo.setDetails(Details.toStdString());
			// save the absolute file path and the file name into the secrets
			visInfo.removeSecret("FileName");
			visInfo.addSecret("FileName", absoluteFilePath_FileName.toStdString());
			this->ref_scn->get()->getDmdEditor().dmdSet_setVisInfo(setID, std::move(visInfo));
		}
		catch (std::exception& e) {
			CreateErrorBoxForCatchE(e);
		}
	}
	else {
		// any row that newly created but without the naming would pop out missing file name error
		CreateErrorBoxForRow(setID, std::string("file name is empty, saving action is failed"));
	}
}

void DSigDemandFileFunction::DemSaveintoFileAndCore(int setID, QString absoluteFilePath_FileName, QString Name, QString Details) {

	if (!absoluteFilePath_FileName.isEmpty()) {
		// save file in, got the file name
		try {
			// save into the core
			auto& dmdSet = this->ref_scn->get()->getDemand().getDemandSet(setID);
			auto visInfo = dmdSet.getVisInfo();
			visInfo.setName(Name.toStdString());
			visInfo.setDetails(Details.toStdString());

			// Copy Secrets out and set it to be Empty first, coz
			visInfo.removeSecret("FileName");

			// overwrite into VisInfo to the editor in core; here Secrets is empty
			this->ref_scn->get()->getDmdEditor().dmdSet_setVisInfo(setID, DISCO2_API::VisInfo{ visInfo });

			// save into the core
			this->ref_scn->get()->saveDemandSet(setID, absoluteFilePath_FileName.toUtf8().constData());

			// save the absolute file path and the file name into the secrets
			visInfo.addSecret("FileName", absoluteFilePath_FileName.toStdString());
			this->ref_scn->get()->getDmdEditor().dmdSet_setVisInfo(setID, std::move(visInfo));
		}
		catch (std::exception& e) {
			CreateErrorBoxForCatchE(e);
		}
	}
	// allows cancel of the file saving dialogue
}

int DSigDemandFileFunction::getIDhaventused() {
	int New_setID ;
	// creating the new setid for the sigset 
	if (this->type == Type_File::Signal) {
		// creating the new Signal_setid 
		New_setID = this->ref_scn->get()->getSigEditor().sigSet_create();
	}
	else if (this->type == Type_File::Demand) {
		// creating the new demand_setid 
		New_setID = this->ref_scn->get()->getDmdEditor().dmdSet_create();
	}

	return New_setID;
}

// --- --- --- --- --- ---  Error Message creation --- --- --- --- --- ---  
void DSigDemandFileFunction::CreateErrorBoxForRow(int ID, std::string ErrorMessage) {
	std::string FullErrorMessage;
	// determine type
	switch (this->type) {
	case Type_File::Signal: {
		FullErrorMessage = std::string("Signal set ID:") + std::to_string(ID) + "\n" + ErrorMessage;
		break;
	}
	case Type_File::Demand: {
		FullErrorMessage = std::string("Demand set ID:") + std::to_string(ID) + "\n" + ErrorMessage;
		break;
	}
	} // end of switch type

	// print out corresponding error
	QMessageBox ErrormessageBox;
	QString Error_name = QString::fromStdString(FullErrorMessage);
	ErrormessageBox.critical(nullptr, "Error", Error_name);
	ErrormessageBox.setFixedSize(500, 200);
}

void DSigDemandFileFunction::CreateErrorBoxForCatchE(std::exception& e) {
	// print out corresponding error
	QMessageBox ErrormessageBox;
	QString Error_name = QString::fromStdString(e.what());
	ErrormessageBox.critical(this->DSigDemandFile, "Error", Error_name);
	ErrormessageBox.setFixedSize(500, 200);
}

// --- --- --- --- --- Slots for Panel --- --- --- --- ---

void DSigDemandFileFunction::dialogFinished() {
	// save the last selected row to be the current setID
	this->setCurrentSelectedSetFile();

	// save all newly created files (signal or demand) before close the this Dialog
	this->saveAllFilesfunc();

	this->DSigDemandFile->close();
	this->closeFunction();
}

void DSigDemandFileFunction::saveAllFilesfunc() {
	// get the selected row
	int total_TableRowCount = this->DSigDemandFile->getUI().TableWidget->rowCount();

	for (int cur_row = 0; cur_row < total_TableRowCount; cur_row++) {

		// getting selected QTableWidgetItem items
		QSpinBox* IDCell = static_cast<QSpinBox*>(this->DSigDemandFile->getUI().TableWidget->cellWidget(cur_row, ColoumnHead::ID));
		QTableWidgetItem* FileNameCell = this->DSigDemandFile->getUI().TableWidget->item(cur_row, ColoumnHead::FNm);
		QTableWidgetItem* NameCell = this->DSigDemandFile->getUI().TableWidget->item(cur_row, ColoumnHead::Nm);
		QTableWidgetItem* DetailsCell = this->DSigDemandFile->getUI().TableWidget->item(cur_row, ColoumnHead::Dtl);

		// getting selected QTableWidgetItem variables
		int setID = IDCell->value();

		QString fileName = FileNameCell->data(Qt::DisplayRole).toString();
		// specific file info for this set file, will be saving internally for avoiding same filename overwriting 
		// default save in the current file dir, but should be keep track of all saved file location to avoid new saving
		// --- --- !!!! fix in future --- --- 
		QFileInfo FileNameInfo(QDir::current(), fileName + QString(".xml"));

		QString Name = NameCell->data(Qt::DisplayRole).toString();
		QString Details = DetailsCell->data(Qt::DisplayRole).toString();

		if (this->type == Type_File::Signal) {
			// save the row wise inf into the core
			//SigSaveintoCore(setID, directory.absoluteFilePath(fileName + QString(".xml")), Name, Details);
			SigSaveintoCore(setID, FileNameInfo.absoluteFilePath(), Name, Details);

		}
		else if (this->type == Type_File::Demand) {
			// save the row wise inf into the core
			//DemSaveintoCore(setID, directory.absoluteFilePath(fileName + QString(".xml")), Name, Details);
			DemSaveintoCore(setID, FileNameInfo.absoluteFilePath(), Name, Details);
		}
	}
}

void DSigDemandFileFunction::setCurrentSelectedSetFile() {

	if (this->DSigDemandFile->getUI().TableWidget->rowCount()) {
		// get the selected row
		int select_rowidx = this->DSigDemandFile->getUI().TableWidget->selectionModel()->currentIndex().row();

		//hot fix when there is not selected
		// eg. delete till nothing on list, load 1 file and close
		// not sure why delete and close works, as there doesn't appear to be have a row selected either
		//	-JLo
		if (select_rowidx == -1)
			select_rowidx = 0;

		// getting selected QTableWidgetItem items
		QSpinBox* IDCell = static_cast<QSpinBox*>(this->DSigDemandFile->getUI().TableWidget->cellWidget(select_rowidx, ColoumnHead::ID));

		// getting selected QTableWidgetItem variables
		int setID = IDCell->value();

		if (this->type == Type_File::Signal) {
			// creating the new Signal_setid 
			this->ref_scn->get()->getSimBuilder().useSigSet(setID);
		}
		else if (this->type == Type_File::Demand) {
			// creating the new demand_setid 
			this->ref_scn->get()->getSimBuilder().useDmdSet(setID);
		}
	}
}