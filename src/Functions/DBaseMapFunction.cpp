#include "DBaseMapFunction.h"

#include "GraphicsItems/DGraphicsView.h"
#include "GraphicsItems/DGraphicsScene.h"
#include "GraphicsItems/DBaseMapTileItem.h"

#include <QScrollBar>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDebug>

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DBaseMapFunction::DBaseMapFunction(I_TileGenerator* tileGenerator) : QObject(), I_Function(), 
	m_tileGenerator(tileGenerator), m_netManager(new QNetworkAccessManager{this}) {}

DBaseMapFunction::~DBaseMapFunction() {}

void DBaseMapFunction::initFunctionHandler() {
	//hopefully these triggers are sufficient
	QObject::connect(this->ref_graphicsView->horizontalScrollBar(), &QAbstractSlider::valueChanged, this, &DBaseMapFunction::sceneMoved);
	QObject::connect(this->ref_graphicsView->verticalScrollBar(), &QAbstractSlider::valueChanged, this, &DBaseMapFunction::sceneMoved);
	QObject::connect(this->ref_graphicsView, &DGraphicsView::graphicsViewResized, this, &DBaseMapFunction::sceneMoved);
	QObject::connect(this->ref_graphicsView, &DGraphicsView::graphicsViewResized, this, &DBaseMapFunction::updateWidgetPos);

	QObject::connect(this->m_netManager, &QNetworkAccessManager::finished, this, &DBaseMapFunction::downloadedTile);
}

void DBaseMapFunction::setTileGenerator(I_TileGenerator* tileGenerator) {
	//make a swap and delete last
	delete this->m_tileGenerator;
	this->m_tileGenerator = tileGenerator;

	delete this->m_overlayWidget;
	this->m_overlayWidget = this->m_tileGenerator->overlayWidget();
	if (this->m_overlayWidget) {
		this->m_overlayWidget->setParent(this->ref_graphicsView);
		this->m_overlayWidget->raise();
		this->m_overlayWidget->show();
		this->updateWidgetPos();
	}

	//clean up all current
	this->m_hasNextRect = false;
	this->m_isCurrentClear = true;
	this->m_downloading.clear();
	this->m_tileCache.clear();
	for (auto item : this->m_tileCacheQueue) {
		this->ref_graphicsScene->removeItem(item);
		delete item;
	}
	this->m_tileCacheQueue.clear();
	
	//refresh the base map
	this->ref_graphicsView->fitInView(this->m_tileGenerator->canvasRect(), Qt::AspectRatioMode::KeepAspectRatio);
	this->sceneMoved();
}

// --- --- --- --- --- Getter Setter --- --- --- --- ---

bool DBaseMapFunction::getIsShowBaseMap() {
	return this->m_showBaseMap;
}

void DBaseMapFunction::setIsShowBaseMap(bool isShow) {
	bool showBefore = this->m_showBaseMap;
	this->m_showBaseMap = isShow;
	//if there is a change
	if (showBefore != this->m_showBaseMap) {
		if (showBefore) {
			//stop any currently downloading from showing
			this->m_hasNextRect = false;
			this->m_hasMoreToDownload = false;
			this->m_downloading.clear();
			//hide all loaded from view
			for (auto& z : this->m_tileCache)
				for (auto& x : z.second)
					for (auto& y : x.second)
						y.second->setVisible(false);
			//hide overlay
			this->m_overlayWidget->hide();
		}
		else {
			//show overlay
			this->m_overlayWidget->show();
			//trigger show basemap iff not showing before
			this->sceneMoved();
		}
	}
}

// --- --- --- --- --- Function Slots --- --- --- --- ---

void DBaseMapFunction::sceneMoved() {
	//don't do anything if there isn't a tileGenerator
	if (!this->m_tileGenerator || !this->m_showBaseMap)
		return;

	//update the rect and get any tiles needed
	this->m_nextRect = this->ref_graphicsView->mapToScene(this->ref_graphicsView->viewport()->rect()).boundingRect();
	if (this->m_isCurrentClear) {
		this->queueRect();
	}
	else {
		this->m_hasNextRect = true;
	}

	//update the current level and hide higher(more detail) level stuff
	int newZoom = this->m_tileGenerator->zoomLevel(this->m_nextRect);
	if (newZoom < this->m_currentZ) {
		for (auto& zThing : this->m_tileCache)
			if (zThing.first > newZoom)
				for (auto& thing : zThing.second)
					for (auto& item : thing.second)
						item.second->setVisible(false);
	}
	this->m_currentZ = newZoom;
}

void DBaseMapFunction::downloadedTile(QNetworkReply* reply) {
	//ignore if not in downloading
	if (!this->m_downloading.count(reply)) {
		this->replyDone(reply);
		return;
	}
	//handle any errors
	QNetworkReply::NetworkError error = reply->error();
	if (error) {
		this->replyDone(reply);
		qDebug() << "QNetworkReply::NetworkError " << reply->errorString();
		//stop further loadings
		this->m_showBaseMap = false;
		return;
	}
	int httpCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
	//try to load into pixMap
	QPixmap pm;
	pm.loadFromData(reply->readAll());
	if (pm.isNull()) {
		this->replyDone(reply);
		qDebug() << "QNetworkRequest::HttpStatusCode " << httpCode;
		//stop further loadings
		this->m_showBaseMap = false;
		return;
	}

	//create the item
	auto& tileInfo = this->m_downloading.at(reply);
	DBaseMapTileItem* item = new DBaseMapTileItem(pm, tileInfo);
	item->setScale(this->m_tileGenerator->tileSceneScale(tileInfo));
	item->setPos(this->m_tileGenerator->tileScenePos(tileInfo));
	//add to scene
	this->ref_graphicsScene->addItem(item);
	this->ref_graphicsScene->update();

	//do the record
	if (!this->m_tileCache.count(tileInfo.z))
		this->m_tileCache.emplace(tileInfo.z, std::unordered_map<int, std::unordered_map<int, DBaseMapTileItem*>>{});
	if (!this->m_tileCache.at(tileInfo.z).count(tileInfo.x))
		this->m_tileCache.at(tileInfo.z).emplace(tileInfo.x, std::unordered_map<int, DBaseMapTileItem*>{});
	this->m_tileCache.at(tileInfo.z).at(tileInfo.x).emplace(tileInfo.y, item);
	this->m_tileCacheQueue.push_back(item);
	//clean the queue;
	if (this->m_tileCacheQueue.size() > this->m_queueMaxSize) {
		//pop from queue
		auto item = this->m_tileCacheQueue.front();
		this->m_tileCacheQueue.pop_front();
		//remove from cache
		const auto& tileinfo = item->getTileInfo();
		this->m_tileCache.at(tileinfo.z).at(tileinfo.x).erase(tileinfo.y);
		//remove and delete
		this->ref_graphicsScene->removeItem(item);
		delete item;
	}

	this->replyDone(reply);
}

void DBaseMapFunction::updateWidgetPos() {
	if (this->m_overlayWidget) {
		auto bottomRight = this->ref_graphicsView->viewport()->rect().bottomRight();
		this->m_overlayWidget->move({ bottomRight.x() - this->m_overlayWidget->width(), bottomRight.y() - this->m_overlayWidget->height() });
	}
}

// --- --- --- --- --- Private Utils --- --- --- --- ---

void DBaseMapFunction::queueRect() {
	QRectF targetRect = this->m_nextRect;
	auto displayTiles = this->m_tileGenerator->genRequiredTileInfos(targetRect);

	//figure what needs to be downloaded
	this->m_needTile.clear();
	for (auto& tile : displayTiles) {
		if (!this->m_tileCache.count(tile.z) || !this->m_tileCache.at(tile.z).count(tile.x) || !this->m_tileCache.at(tile.z).at(tile.x).count(tile.y)) {
			this->m_needTile.push_back(tile);
		}
		else {
			//ensure the exsisting tiles are visible
			DBaseMapTileItem* tileItem = this->m_tileCache.at(tile.z).at(tile.x).at(tile.y);
			tileItem->setVisible(true);
			//refresh its place in the queue
			for (auto itr = this->m_tileCacheQueue.begin(); itr != this->m_tileCacheQueue.end(); ++itr) {
				if (*itr == tileItem) {
					this->m_tileCacheQueue.erase(itr);
					break;
				}
			}
			this->m_tileCacheQueue.push_back(tileItem);
		}
	}

	//create the requests
	this->requestTiles();
}

void DBaseMapFunction::requestTiles() {
	//create the requests
	int requestCount = 0;
	while (!this->m_needTile.empty() && requestCount < this->m_requestLimit) {
		auto tile = this->m_needTile.front();
		QNetworkRequest request(this->m_tileGenerator->buildUrl(tile));
		QNetworkReply* reply = this->m_netManager->get(request);
		this->m_downloading.emplace(reply, std::move(tile));
		this->m_needTile.pop_front();
		++requestCount;
	}
	this->m_hasMoreToDownload = !this->m_needTile.empty();

	if (requestCount)	//requests made so is downloading
		this->m_isCurrentClear = false;
}

void DBaseMapFunction::replyDone(QNetworkReply* reply) {
	this->m_downloading.erase(reply);
	reply->deleteLater();

	//trigger next round of loading if all downloaded
	if (this->m_downloading.empty()) {
		if (this->m_hasNextRect) {		//has new rect than redo everything using the new rect
			this->m_isCurrentClear = true;
			this->queueRect();
			this->m_hasNextRect = false;
		}
		else if (this->m_hasMoreToDownload) {
			this->requestTiles();
		}
		else {
			this->m_isCurrentClear = true;
		}
	}
}
