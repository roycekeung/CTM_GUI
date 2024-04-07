#pragma once

#include <list>
#include <unordered_set>
#include <unordered_map>

#include <QGraphicsScene>

#include <Scenario.h>

#include "..\Utils\CoordTransform.h"

class DCellItem;
class DPolyArcItem;
class DJctItem;
class DDemandSinkCellItem;
class DCellConnectorItem;
class DPseudoJctLinkItem;
class DJctLinkItem;

class DMainWindow;

/**
 Manages all the DISCO graphics items
*/
class DGraphicsScene : public QGraphicsScene {
	Q_OBJECT

public:

	const static double laneWidth;

	//Default z-values (layer, higher is higher layer) for items
	enum ZValue_GraphicItemLayer {
		BaseMap = 0, 
		JctItem = 60,
		ArcItem = 70,								//reserve z-values for arc to implement paint?
		CellItem = 80, DmdSnkCellItem = 99,			//reserve z-values for arc to order overlapping cells
		CellConnectorItem = 100,					//reserve z-values for cell to order overlapping connectors
		PseudoJLinkItem = 110,						//reserve z-values for jct to order overlapping JLinks
		JLinkItem = 120,
		ArcSpinePt = 130
	};

private:

	std::shared_ptr<DISCO2_API::Scenario>* ref_scn;
	DMainWindow* ref_mainWindow;

	CoordTransform* ref_coordTransform;

	// --- --- --- --- --- Graphic Items --- --- --- --- ---
	// - list of items of same type

	std::unordered_set<int> ref_dmdsnkCellIds;			//plain copy from core net
	std::unordered_set<int> ref_cellIds;				//only cells that are not within JLinks
	std::unordered_set<int> ref_arcIds;					//only arcs that are not JLinks
	std::unordered_set<int> ref_jctIds;					//plain copy from core net
	std::unordered_set<int> ref_jctLinkArcIds;			//extracted from jcts
	std::unordered_map<int, int> ref_jctInOutCellId;	//extracted from jcts (key: cellId; value: jctId)

	std::unordered_map<int, DDemandSinkCellItem*> m_DmdSnkCellItems;
	std::unordered_map<int, DCellItem*> m_CellItems;
	std::unordered_map<int, DPolyArcItem*> m_ArcItems;
	std::unordered_map<int, DJctItem*> m_JctItems;
	std::unordered_map<int, DJctLinkItem*> m_JctLinkItems;		//key: ArcId -> key: DJctLinkItem 

	std::unordered_map<int, std::unordered_map<int, std::unordered_map<int, DCellItem*>>> m_JCellItems;		//key: JctId -> key: JLinkId -> key: CellId
	std::unordered_map<int, std::unordered_map<int, DJctLinkItem*>> m_JLinkItems;						//key: JctId -> key: JLinkId  
	std::unordered_map<int, std::unordered_map<int, DPseudoJctLinkItem*>> m_PseudoJLinkItems;		//key: JctId -> key: JLinkId

	struct CellConnectorEntry{
		int fromCellId = -1;
		int toCellId = -1;
		DCellConnectorItem* item = nullptr;

		bool comp(int& fromCellId, int& toCellId) const { return (this->fromCellId == fromCellId && this->toCellId == toCellId); }
		bool hasCellId(int& toCheck) const { return (this->fromCellId == toCheck || this->toCellId == toCheck); }
	};
	std::list<CellConnectorEntry> m_CellConnectorItems;

	struct CellJctConnectorEntry {
		int jctId = -1;
		int cellId = -1;
		DCellConnectorItem* item = nullptr;

		bool comp(int& jctId, int& cellId) const { return (this->jctId == jctId && this->cellId == cellId); }
		bool hasCellId(int& toCheck) const { return (this->cellId == toCheck); }
		bool hasJctId(int& toCheck) const { return (this->jctId == toCheck); }
	};
	std::list<CellJctConnectorEntry> m_CellJctConnectorItems;

	// --- --- --- --- --- Display States --- --- --- --- ---
	// - For keeping track of what sort of behaviour should happen
	bool mStat_showingCell2CellConnector = false;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DGraphicsScene(DMainWindow* mainWindow, std::shared_ptr<DISCO2_API::Scenario>* scn, CoordTransform* coordTransform);

	~DGraphicsScene();

	// --- --- --- --- --- Getters --- --- --- --- ---
	const std::unordered_set<int>& getRefDmdsnkCellIds() const;
	const std::unordered_set<int>& getRefCellIds() const;
	const std::unordered_set<int>& getRefArcIds() const;
	const std::unordered_set<int>& getRefJctIds() const;
	const std::unordered_set<int>& getRefJctLinkArcIds() const;
	const std::unordered_map<int, int>& getRefJctInOutCellId() const;

	DCellItem* getDCellItem(int cellId);
	DPolyArcItem* getDArcItem(int arcId);
	DJctItem* getDJctItem(int jctId);
	DDemandSinkCellItem* getDmdSnkCellItem(int cellId);
	const std::unordered_map<int, DPseudoJctLinkItem*>& getPseudoJctLinksInJct(int jctId) const;
	const std::unordered_map<int, DJctLinkItem*>& getJctLinksInJct(int jctId) const;
	const std::unordered_map<int, DCellItem*>& getJctLinkCellsInJct(int jctId, int jctLinkId) const;

	DCellConnectorItem* getDCellConnector_C2C(int fromCellId, int toCellId);
	std::unordered_map<int, DCellConnectorItem*> getToCellConnectors(int fromCellId);
	std::unordered_map<int, DCellConnectorItem*> getFromCellConnectors(int toCellId);

	CellConnectorEntry getCellConnectorEntry_C2C(DCellConnectorItem* connector) const;
	CellJctConnectorEntry getCellConnectorEntry_C2J(DCellConnectorItem* connector) const;

public slots:

	// --- --- --- --- --- Handling Graphic Items --- --- --- --- ---
	
	/**
	 clears all exsisting drawn items
	 reads from ref-scn
	*/
	void reloadNet();
	void reloadLists();

	//Creates the DCellItem
	DCellItem* addDCellItem(int cellId);
	//Creates the DJctItem
	DJctItem* addDJctItem(int jctId);
	//Creates the DArcItem, and also all the DCellItem it contains
	DPolyArcItem* addDArcItem(int arcId);
	//Creates the DDemandSinkCellItem whereas read from core
	DDemandSinkCellItem* addDmdSnkItem(int cellId);
	//Creates the DCellConnector between 2 cells, must be ordered
	DCellConnectorItem* addDCellConnector_C2C(int fromCellId, int toCellId);
	//Creates a DCellConnector as pseudo connection in/out a jct, will auto config direction
	DCellConnectorItem* addDCellConnector_C2J(int cellId, int jctId);
	//Creates all DPseudoJLink in a jct
	void addAllDPseudoJLinkItemInJct(int jctId); 
	//Creates all the DArc and DCell items for all the JLink in a jct
	void addAllJLinkItemInJct(int jctId);   

	//Removes the DCellItem from scene
	void removeDCellItem(int cellId);	
	//Removes the DJctItem from scene
	void removeDJctItem(int jctId);		
	//Removes the DArcItem and all the DCellItem it contains from scene
	void removeDArcItem(int arcId);		
	//Removes the DDemandSinkCellItem from scene
	void removeDmdSnkItem(int cellId);	

	void addConnectorsOfSelectedItems();
	void addAllConnectorOfVisibleItems();

	void removeDCellConnector_C2C(int fromCellId, int toCellId);
	void removeDCellConnector_C2J(int cellId, int jctId);
	void removeAllDPseudoJLinkItemInJct(int jctId);  
	void removeAllJLinkItemInJct(int jctId);  
	void removeAllDCellConnector();
	void removeAllDCellConnector_C2C();
	void removeAllDCellConnector_C2J();

	//Updates the location and size of the DCellItem -> read from visInfo
	void updateDCellItem(int cellId);
	//Updates the location and sizes of the DArcItem and updates all the DCellItems "within" 
	void updateDArcItem(int arcId);
	//Updates the location and sizes of the DJctItem
	void updateDJctItem(int jctId);
	//Updates the location and size of the DmdSnkcellItem -> read from visInfo
	void updateDmdSnkItem(int cellId);	
	
	// --- --- --- --- --- Show/Hide Graphic Items --- --- --- --- ---

	//Changes visibility of all DDmdSnkCellItem
	void setShowAllDmdSnkCell(bool isShow);
	//Changes visibility of all DCellItem
	void setShowAllCell(bool isShow);
	//Changes visibility of all DArcItem
	void setShowAllArc(bool isShow);
	//Changes visibility of all DJctItem
	void setShowAllJct(bool isShow);

	//Convenience Function to switch all existing JLinks visibility 
	void setShowDetailJctLinkView(int jctId, bool isShowDetail);
	//Switches the visibility of JLink between Pseudo & Arc+Cell if exists
	void setShowDetailJctLinkView(int jctId, int jctLinkId, bool isShowDetail);

	//Convenience Function to rest all cell's color to default
	void resetAllCellColor();
	void resetAllCellText();

signals:
	// any add or del graphicitems will triger this
	void itemsChanged();

private:

	// --- --- --- --- --- Private Utils --- --- --- --- ---

	//Util function: Calculates the centroid location and rotation of all the cells coords within an arc
	void calArcCellCoords(int arcId);	
};
