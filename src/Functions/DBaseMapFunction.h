#pragma once

#include <List>

#include <QObject>

#include "I_Function.h"
#include "Utils/I_TileGenerator.h"

class DBaseMapTileItem;
class QNetworkAccessManager;
class QNetworkReply;

class DBaseMapFunction : public QObject, public I_Function {
	Q_OBJECT
private:

	bool m_showBaseMap = true;

	I_TileGenerator* m_tileGenerator;
	QWidget* m_overlayWidget = nullptr;
	QNetworkAccessManager* m_netManager; 

	bool m_hasNextRect = false;
	QRectF m_nextRect;

	bool m_isCurrentClear = true;
	bool m_hasMoreToDownload = false;
	std::list<TileInfo> m_needTile;
	std::unordered_map<QNetworkReply*, TileInfo> m_downloading;
	const int m_requestLimit = 8;

	int m_currentZ = -1;

	const int m_queueMaxSize = 16384;	//higher cache to reduce map calls -JLo
	std::list<DBaseMapTileItem*> m_tileCacheQueue;	//FIFO queue
	std::unordered_map<int, std::unordered_map<int, std::unordered_map<int, DBaseMapTileItem*>>> m_tileCache;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DBaseMapFunction(I_TileGenerator* tileGenerator = nullptr);

	~DBaseMapFunction();

	void initFunctionHandler() override;

	void setTileGenerator(I_TileGenerator* tileGenerator);

	// --- --- --- --- --- Getter Setter --- --- --- --- ---

	bool getIsShowBaseMap();

	void setIsShowBaseMap(bool isShow);

public slots:

	// --- --- --- --- --- Function Slots --- --- --- --- ---

	void sceneMoved();

	void downloadedTile(QNetworkReply* reply);

	//hardcodes the outlay to be bottom right
	void updateWidgetPos();

private:

	// --- --- --- --- --- Private Utils --- --- --- --- ---

	// Process a scene rect, ensure tiles are visible or start downloading them
	void queueRect();

	// create and send the download requests, each call limits requests by m_requestLimit
	void requestTiles();

	// Removes QNetworkReply from downloading and deletes it
	void replyDone(QNetworkReply* reply);

};
