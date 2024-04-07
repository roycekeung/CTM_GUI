#include "DDemandSinkCellEditFunc.h"

//DISCO GUI stuff
#include "PanelWidgets/Network/DDemandSinkCellPanel.h"
#include "GraphicsItems/DDemandSinkCellItem.h"
#include "DDockWidgetHandler.h"
#include "GraphicsItems/DGraphicsScene.h"
#include "Utils/MouseWheelWidgetAdjustmentGuard.h"
#include "Utils/DDefaultParamData.h"
#include "Functions/DBaseNaviFunction.h"
#include "DAddRemoveButtons.h"

//Qt lib
#include <QGraphicsSceneMouseEvent>
#include <QPushButton>
#include <QKeyEvent>
#include <QComboBox>

//DISCO Core lib
#include "Cell_DemandSink.h"
#include "VisInfo.h"

// --- --- --- --- --- internal Functions --- --- --- --- ---

void DDemandSinkCellEditFunc::backupTableModel() {
	if (this->ref_scn->get()->getDemand().getDemandSetIds().empty()) {
		emit SendVisibleSignalToAddRemoveButton(false);
		return;
	}
	// save the 1st dmdsetID
	this->previous_dmdsetID = *this->ref_scn->get()->getDemand().getDemandSetIds().begin();

	// back up all loader from core to 
	for (int DemSetID : this->ref_scn->get()->getDemand().getDemandSetIds()) {
		if (this->ref_scn->get()->getDemand().getDemandSet(DemSetID).hasDmdLoader(this->tmp_CellID) == false) {
			// dmdset dont have the demand loader yet
			// so pass this setID 
		}
		else {
			auto DemandLoaders_inf = this->ref_scn->get()->getDemand().getDemandSet(DemSetID).getAllIntervals_continuousDmdLoad(this->tmp_CellID);
			this->tmp_model.insert(std::pair<int, std::vector< std::tuple<int64_t, int64_t, double>>>(DemSetID, DemandLoaders_inf));

		}
	}
	// table had successfully backed up
}

void DDemandSinkCellEditFunc::showDemandInputTable() {
	if (this->ref_scn->get()->getDemand().getDemandSetIds().empty()) {
		emit SendVisibleSignalToAddRemoveButton(false);
		return;
	}

	for (int DemSetID : this->ref_scn->get()->getDemand().getDemandSetIds()) {
		this->m_panelWidet->getUI().Plan_comboBox->addItem(QString("%1").arg(DemSetID));
	}
}

void DDemandSinkCellEditFunc::saveAllDemandLoadingintoCore_perSet(int DemSetID) {
	//  true if tmp_model conclude, 
	if (this->m_panelWidet->getUI().Plan_comboBox->currentText().toInt() != DemSetID && this->tmp_model.count(DemSetID)) {
		for (int cur_row = 0; cur_row < this->tmp_model.at(DemSetID).size(); cur_row++) {

			// save continuous load into Core ; data reading from the tmp table backup
			this->ref_scn->get()->getDmdEditor().dmdLoad_continuous_addInterval(DemSetID, this->tmp_CellID,
				std::get<0>(this->tmp_model.at(DemSetID).at(cur_row)),
				std::get<1>(this->tmp_model.at(DemSetID).at(cur_row)),
				std::get<2>(this->tmp_model.at(DemSetID).at(cur_row)));
		}
		return;
	}
	else if (this->m_panelWidet->getUI().Plan_comboBox->currentText().toInt() == DemSetID) {
		// false, if not then save data from the current qttablewidget
		// get the selected row
		int total_TableRowCount = this->m_panelWidet->getUI().DemandInput_tableWidget->rowCount();

		for (int cur_row = 0; cur_row < total_TableRowCount; cur_row++) {

			// getting selected QTableWidgetItem items
			QSpinBox* startCell = static_cast<QSpinBox*>(this->m_panelWidet->getUI().DemandInput_tableWidget->cellWidget(cur_row, 0));
			QSpinBox* endCell = static_cast<QSpinBox*>(this->m_panelWidet->getUI().DemandInput_tableWidget->cellWidget(cur_row, 1));
			QDoubleSpinBox* rateCell = static_cast<QDoubleSpinBox*>(this->m_panelWidet->getUI().DemandInput_tableWidget->cellWidget(cur_row, 2));

			// getting selected QTableWidgetItem variables
			int startTime = startCell->value();
			int endTime = endCell->value();
			double rate = rateCell->value()/3600;

			// save continuous load into Core
			this->ref_scn->get()->getDmdEditor().dmdLoad_continuous_addInterval(DemSetID, this->tmp_CellID, startTime, endTime, rate);
		}
	}
}

DDemandSinkCellEditFunc::DDemandSinkCellEditFunc(int cellId) : tmp_CellID(cellId), QObject(nullptr) {

}

DDemandSinkCellEditFunc::~DDemandSinkCellEditFunc() {
	
}


// --- --- --- --- --- override initFunctionHandler from I_Function --- --- --- --- ---
void DDemandSinkCellEditFunc::initFunctionHandler() {
	// NO MORE SCROLLING
	this->noMoreScrolling = new MouseWheelWidgetAdjustmentGuard{ this };

	const DISCO2_API::Cell_DemandSink* m_DmdSnkCellFromCore = this->ref_scn->get()->getNetwork().getCell_DemandSink(this->tmp_CellID);
	
	// ensure intact selection of the Demand Sink Cell graphicitem then maybe plot with connecter 
	this->ref_graphicsScene->removeAllDCellConnector();
	this->ref_graphicsScene->clearSelection();
	this->ref_graphicsScene->getDmdSnkCellItem(this->tmp_CellID)->setSelected(true);
	this->ref_graphicsScene->addConnectorsOfSelectedItems();

	//grab the item that's clicked on
	DDemandSinkCellItem* topItem = dynamic_cast<DDemandSinkCellItem*>(this->ref_graphicsScene->selectedItems().first());

	//setup the widget; by double confirmation; getFlowInCapacity check got some flaws; but force flow cap input has to be non-0 then solved 
	if (m_DmdSnkCellFromCore->getFlowInCapacity() == 0 && topItem->getTypeOfCell()== DDemandSinkCellItem::CellType::Demand) {
		this->m_panelWidet = new DDemandSinkCellPanel{ DDemandSinkCellPanel::Type_Panel::edit, this->ref_dParam, DDemandSinkCellPanel::CellType::Demand, m_DmdSnkCellFromCore,
		this->ref_graphicsScene->getToCellConnectors(this->tmp_CellID), this->ref_graphicsScene->getFromCellConnectors(this->tmp_CellID) };
		this->ref_dockWidgetHandler->setRightDockWidget(this->m_panelWidet, "Edit Demand Cell");
	}
	else if ((m_DmdSnkCellFromCore->getFlowInCapacity() != 0 && topItem->getTypeOfCell() == DDemandSinkCellItem::CellType::Sink)) {
		this->m_panelWidet = new DDemandSinkCellPanel{ DDemandSinkCellPanel::Type_Panel::edit, this->ref_dParam, DDemandSinkCellPanel::CellType::Sink, m_DmdSnkCellFromCore,
		this->ref_graphicsScene->getToCellConnectors(this->tmp_CellID), this->ref_graphicsScene->getFromCellConnectors(this->tmp_CellID) };
		this->ref_dockWidgetHandler->setRightDockWidget(this->m_panelWidet, "Edit Sink Cell");
	}

	// setup and connect to panel
	this->m_panelWidet->connectEditFunction(this);

	// setup and DemandInputTable widget
	showDemandInputTable();

	// get the demand sink cell item back up from graphicscene
	this->m_CellItem = this->ref_graphicsScene->getDmdSnkCellItem(this->tmp_CellID);
	// set it back to the edit mode alows rotation and translation
	this->m_CellItem->setMovableRotatable(true);
	this->m_panelWidet->connectItemToConfirmButton(this->m_CellItem);

	// disable the baseNaviFunction
	this->ref_baseNaviFunction->setIgnoreEvents(true);

}


// --- --- --- --- --- update and Edit Functions --- --- --- --- ---
void DDemandSinkCellEditFunc::addEmptyRow() {
	// change the sender signal back to the customized DAddRemoveButtons widget object class
	QPushButton* SenderButton = qobject_cast<QPushButton*>(sender());

	// TableRowCount in UI
	int TableRowCount = this->m_panelWidet->getUI().DemandInput_tableWidget->rowCount();

	// add one more empty row
	this->m_panelWidet->getUI().DemandInput_tableWidget->setRowCount(std::max(1, TableRowCount + 1));

	// to determine which table button is clicked
	QString ObjName = this->m_panelWidet->getUI().DemandInput_tableWidget->objectName();
	if (ObjName == QString::fromUtf8("DemandInput_tableWidget")) {
		// defualt 1st column initial set up
		QSpinBox* startTimeCell = new QSpinBox(this->m_panelWidet->getUI().DemandInput_tableWidget);
		startTimeCell->setMaximum(INT_MAX);
		startTimeCell->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
		startTimeCell->setButtonSymbols(QAbstractSpinBox::NoButtons);
		this->m_panelWidet->getUI().DemandInput_tableWidget->setCellWidget(std::max(0, TableRowCount), 0, startTimeCell);
		startTimeCell->setToolTip(tr("New"));
		startTimeCell->setReadOnly(false);  // editable and selectable
		startTimeCell->setFrame(false);  // no frame
		startTimeCell->installEventFilter(this->noMoreScrolling);
		// defualt 2nd column initial set up
		QSpinBox* endTimeCell = new QSpinBox(this->m_panelWidet->getUI().DemandInput_tableWidget);
		endTimeCell->setMaximum(INT_MAX);
		endTimeCell->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
		endTimeCell->setButtonSymbols(QAbstractSpinBox::NoButtons);
		this->m_panelWidet->getUI().DemandInput_tableWidget->setCellWidget(std::max(0, TableRowCount), 1, endTimeCell);
		endTimeCell->setToolTip(tr("New"));
		endTimeCell->setReadOnly(false);  // editable and selectable
		endTimeCell->setFrame(false);  // have frame
		endTimeCell->installEventFilter(this->noMoreScrolling);
		// defualt 3th column initial set up
		QDoubleSpinBox* rateCell = new QDoubleSpinBox(this->m_panelWidet->getUI().DemandInput_tableWidget);
		rateCell->setMaximum(DBL_MAX);
		rateCell->setDecimals(2);
		rateCell->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
		rateCell->setButtonSymbols(QAbstractSpinBox::NoButtons);
		this->m_panelWidet->getUI().DemandInput_tableWidget->setCellWidget(std::max(0, TableRowCount), 2, rateCell);
		rateCell->setToolTip(tr("New"));
		rateCell->setReadOnly(false);  // editable and selectable
		rateCell->setFrame(false);  // have frame
		rateCell->installEventFilter(this->noMoreScrolling);

	}
}

void DDemandSinkCellEditFunc::removerow() {
	// delete row file and also delete that of record according to the corresponding setID

	// change the sender signal back to the customized DAddRemoveButtons widget object class
	QPushButton* SenderButton = qobject_cast<QPushButton*>(sender());

	// empty table activate this
	int total_TableRowCount = this->m_panelWidet->getUI().DemandInput_tableWidget->rowCount();
	if (total_TableRowCount == 0) {
		return;
	}

	// get the selected row
	int rowID = 0;
	for (int loopCount = 0; rowID < total_TableRowCount; loopCount++) {

		if (this->m_panelWidet->getUI().DemandInput_tableWidget->selectionModel()->isRowSelected(rowID)) {

			// remove from the UI table
			this->m_panelWidet->getUI().DemandInput_tableWidget->removeRow(rowID);

			// stay on same row coz this row is deleted, the following would shift upwards
		}
		else {
			// move to next row
			rowID++;
		}
	}
}

void DDemandSinkCellEditFunc::updateDemandInputTablefromCore(const QString& dmd_setID) {
	// enum CellType { Demand =0 , Sink =1};
	if (this->m_panelWidet->getCellTypeInput() == 1) {
		// Sink Cell; dont have DemandInputTable
		return;
	} 

	// coz this func will comes in construct first before initFunctionHandler(), but after DDemandSinkCellEditFunc construct
	// only initiate once at first
	if (this->tmp_model.empty() && this->m_panelWidet->getUI().DemandInput_tableWidget->rowCount() == 0) {
		//// back up all loader from core to 
		backupTableModel();
	}

	// update previous changes on previous dmdsetID
	if (this->m_panelWidet->getUI().DemandInput_tableWidget->rowCount() == 0) {
		//initial table is empty then do nth
	}
	else {
		// store up the previous changes on previous dmdsetID
		// also delete the original value dmdset backup, which means this is being ammended so not conclude in backup table 
		this->tmp_model.erase(this->previous_dmdsetID);
		int total_TableRowCount = this->m_panelWidet->getUI().DemandInput_tableWidget->rowCount();
		std::vector<std::tuple<int64_t, int64_t, double>> tmp_vtr{};
		for (int cur_row = 0; cur_row < total_TableRowCount; cur_row++) {

			// getting selected QTableWidgetItem items
			QSpinBox* startCell = static_cast<QSpinBox*>(this->m_panelWidet->getUI().DemandInput_tableWidget->cellWidget(cur_row, 0));
			QSpinBox* endCell = static_cast<QSpinBox*>(this->m_panelWidet->getUI().DemandInput_tableWidget->cellWidget(cur_row, 1));
			QDoubleSpinBox* rateCell = static_cast<QDoubleSpinBox*>(this->m_panelWidet->getUI().DemandInput_tableWidget->cellWidget(cur_row, 2));

			// getting selected QTableWidgetItem variables
			int startTime = startCell->value();
			int endTime = endCell->value();
			double rate = rateCell->value()/3600;
			
			tmp_vtr.emplace_back(std::make_tuple(startTime, endTime, rate));
		}
		this->tmp_model.insert(std::pair<int, std::vector< std::tuple<int64_t, int64_t, double>>> (this->previous_dmdsetID , tmp_vtr));
	}

	if (this->tmp_model.count(dmd_setID.toInt())) {
		// demand cell
		auto DemandLoaders_inf = this->tmp_model.at(dmd_setID.toInt());
		/// remove all item data then re-build
		this->m_panelWidet->getUI().DemandInput_tableWidget->setRowCount(0);

		// define the table row number
		this->m_panelWidet->getUI().DemandInput_tableWidget->setRowCount(DemandLoaders_inf.size());

		int current_row = 0;
		for (auto DemandLoader : DemandLoaders_inf) {
			int start_time = std::get<0>(DemandLoader);
			int end_time = std::get<1>(DemandLoader);
			double rate = std::get<2>(DemandLoader);

			// defualt table initial set up
			QSpinBox* startTimeCell = new QSpinBox(this->m_panelWidet->getUI().DemandInput_tableWidget);
			startTimeCell->setMaximum(INT_MAX);  // this->ref_scn->get()->getSimBuilder().getSimTimeSteps() this would be in risk when create new network coz initial simtime is 0
			startTimeCell->setValue(start_time);
			startTimeCell->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
			startTimeCell->setButtonSymbols(QAbstractSpinBox::NoButtons);
			this->m_panelWidet->getUI().DemandInput_tableWidget->setCellWidget(current_row, 0, startTimeCell);
			startTimeCell->setToolTip(tr("load from core"));
			startTimeCell->setReadOnly(false);  // not editable but can select
			startTimeCell->setFrame(false);  // no frame
			startTimeCell->installEventFilter(this->noMoreScrolling);

			QSpinBox* endTimeCell = new QSpinBox(this->m_panelWidet->getUI().DemandInput_tableWidget);
			endTimeCell->setMaximum(INT_MAX);   // this->ref_scn->get()->getSimBuilder().getSimTimeSteps() this would be in risk when create new network coz initial simtime is 0
			endTimeCell->setValue(end_time);
			endTimeCell->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
			endTimeCell->setButtonSymbols(QAbstractSpinBox::NoButtons);
			this->m_panelWidet->getUI().DemandInput_tableWidget->setCellWidget(current_row, 1, endTimeCell);
			endTimeCell->setToolTip(tr("load from core"));
			endTimeCell->setReadOnly(false);  // not editable but can select
			endTimeCell->setFrame(false);  // no frame
			endTimeCell->installEventFilter(this->noMoreScrolling);

			QDoubleSpinBox* rateCell = new QDoubleSpinBox(this->m_panelWidet->getUI().DemandInput_tableWidget);
			rateCell->setMaximum(DBL_MAX);
			rateCell->setValue(rate*3600);
			rateCell->setDecimals(2);
			rateCell->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
			rateCell->setButtonSymbols(QAbstractSpinBox::NoButtons);
			this->m_panelWidet->getUI().DemandInput_tableWidget->setCellWidget(current_row, 2, rateCell);
			rateCell->setToolTip(tr("load from core"));
			rateCell->setReadOnly(false);  // not editable but can select
			rateCell->setFrame(false);  // no frame
			rateCell->installEventFilter(this->noMoreScrolling);

			current_row++;
		}

	}
	else {
		// which means the current selected dmdset is not empty so the local tmp_model dont contain the corresponding dmdset record
		// thus clear the table that left by the previous dmdset
		int total_TableRowCount = this->m_panelWidet->getUI().DemandInput_tableWidget->rowCount();

		for (int cur_row = 0; cur_row < total_TableRowCount; cur_row++) {
			this->m_panelWidet->getUI().DemandInput_tableWidget->removeRow(0);
		}
	}

	// update the current selected combobox dmdsetID
	this->previous_dmdsetID = dmd_setID.toInt();
}


void DDemandSinkCellEditFunc::checkAndEdit() {
	//value checking for mrg div
	const DISCO2_API::Cell_DemandSink* m_DmdSnkCellFromCore = this->ref_scn->get()->getNetwork().getCell_DemandSink(this->tmp_CellID);
	
	if (m_DmdSnkCellFromCore->getToCellIds().size() > 1 && this->ref_graphicsScene->getToCellConnectors(this->tmp_CellID).size() > 1) {
		double sum = 0;
		for (auto& rec : this->m_panelWidet->getDivTableInput())
			sum += rec.second;
		if (std::fabs(sum - 1) > std::numeric_limits<double>::epsilon()) {
			this->m_panelWidet->failMessage({ "Sum of Diverge Ratio must be 1" });
			//early exit as operation failed
			return;
		}
	}
	if (m_DmdSnkCellFromCore->getFromCellIds().size() > 1 && this->ref_graphicsScene->getFromCellConnectors(this->tmp_CellID).size() > 1) {
		double sum = 0;
		for (auto& rec : this->m_panelWidet->getMrgTableInput())
			sum += rec.second;
		if (std::fabs(sum - 1) > std::numeric_limits<double>::epsilon()) {
			this->m_panelWidet->failMessage({ "Sum of Merge Ratio must be 1" });
			//early exit as operation failed
			return;
		}
	}

	// try to override cell inf to core
	try {
		// enum CellType { Demand =0 , Sink =1};
		if (this->m_panelWidet->getCellTypeInput() == 0) {
			// demand cell flow out; outQ
			this->ref_scn->get()->getDmdEditor().dmdSnkCell_overRideFlowCapacity(this->tmp_CellID, 0, this->m_panelWidet->getFlowCapInput());

			/// TODO future coding ; input the Demand either continuous(uniform) or non-uniform loading into core
			// get each number of DemSetID 
			for (int DemSetID : this->ref_scn->get()->getDemand().getDemandSetIds()) {

				if (this->ref_scn->get()->getDemand().getDemandSet(DemSetID).hasDmdLoader(this->tmp_CellID) == false) {
					// dmdset dont have the demand loader yet
					// so create demand loader for the corresponding cell according to each dmdsetID
					this->ref_scn->get()->getDmdEditor().dmdLoad_continuous_create(DemSetID, this->tmp_CellID);
				}
				else {
					// dummy way to restore the amended value demand input into core loader; clear all inf before addin new intervals
					this->ref_scn->get()->getDmdEditor().dmdLoad_continuous_removeAllIntervals(DemSetID, this->tmp_CellID);
				}
				// remove evry loading in core first before add in new amended intervals
				saveAllDemandLoadingintoCore_perSet(DemSetID);
			}
		}
		else if (this->m_panelWidet->getCellTypeInput() == 1) {
			// flow into sink cell; inQ
			this->ref_scn->get()->getDmdEditor().dmdSnkCell_overRideFlowCapacity(this->tmp_CellID, this->m_panelWidet->getFlowCapInput(), 0);
			// Sink Cell; dont have DemandInputTable
		}

		//update the mrg div ratios
		if (m_DmdSnkCellFromCore->getToCellIds().size() > 1 && this->ref_graphicsScene->getToCellConnectors(this->tmp_CellID).size() > 1)
			for (auto& rec : this->m_panelWidet->getDivTableInput())
				this->ref_scn->get()->getNetEditor().cell_setDivergeRatio(this->tmp_CellID, rec.first, rec.second);
		if (m_DmdSnkCellFromCore->getFromCellIds().size() > 1 && this->ref_graphicsScene->getFromCellConnectors(this->tmp_CellID).size() > 1)
			for (auto& rec : this->m_panelWidet->getMrgTableInput())
				this->ref_scn->get()->getNetEditor().cell_setMergeRatio(rec.first, this->tmp_CellID, rec.second);

		// save newly amended inputted params to default data struct bakcup
		this->ref_dParam->m_DmdCellFlowParams.SaturationFlow = this->m_panelWidet->getFlowCapInput();

		//saving new amended visInfo, pos and rotation , name, detail
		DISCO2_API::VisInfo vis{};
		vis.setType(DISCO2_API::VisInfo::Type_VisInfo::Point);
		QPointF tPt1 = this->ref_coordTransform->transformToReal(this->m_CellItem->scenePos());
		DISCO2_API::VisInfo::VisInfo_Pt tVisInfoPt{ tPt1.x(), tPt1.y(), this->m_CellItem->rotation() };
		tVisInfoPt.m_Scale = this->m_CellItem->getScale();
		vis.addPoint(std::move(tVisInfoPt));
		vis.setName(this->m_panelWidet->getNameInput().toStdString());
		vis.setDetails(this->m_panelWidet->getDetailsInput().toStdString());
		this->ref_scn->get()->getNetEditor().cell_setVisInfo(this->tmp_CellID, std::move(vis));

		//close if succeeded
		this->cancel();
	}
	catch (std::exception& e) {
		//print fail msg in the widget
		this->m_panelWidet->failMessage({ e.what() });
	}
}

void DDemandSinkCellEditFunc::cancel() {
	// set it back to the edit mode alows rotation and translation
	this->m_CellItem->setMovableRotatable(false);
	//revert / load the new rotation & size
	this->ref_graphicsScene->updateDmdSnkItem(this->tmp_CellID);

	// clear selection and connector shown manually
	this->ref_graphicsScene->removeAllDCellConnector();
	this->ref_graphicsScene->clearSelection();

	//remove the demand sink cell panel
	this->ref_dockWidgetHandler->removeAndDeleteRightDockWidget();

	// re-able the baseNaviFunction
	this->ref_baseNaviFunction->setIgnoreEvents(false);

	// end this function
	this->closeFunction();
}


// --- --- --- --- --- Re-implement functions for intercepting DGraphicsScene events --- --- --- --- ---
bool DDemandSinkCellEditFunc::keyPressEvent(QKeyEvent* keyEvent) {
	if (keyEvent->key() == Qt::Key::Key_Escape) {
		keyEvent->accept();
		this->cancel();
		return true;
	}
	else if (keyEvent->key() == Qt::Key::Key_Enter) {
		checkAndEdit();
		keyEvent->accept();
		return true;
	}
	// continue to handle other events
	return false;
}


bool DDemandSinkCellEditFunc::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	//intercepts all scene event so notthing else gets selected
	mouseEvent->accept();

	// continue to handle other events
	return false;
}

