#include "DRunManager.h"

#include <I_RecordsHandler.h>
#include <VisInfo.h>

// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

DRunManager::DRunManager() {}

DRunManager::~DRunManager() {
	//have to delete the recs
	for (auto& runResult : this->m_results)
		delete runResult.rec;
}

// --- --- --- --- --- Getters --- --- --- --- ---

size_t DRunManager::size() {
	return this->m_results.size();
}

RunRec* DRunManager::getRunRec(size_t index) {
	if (this->m_results.size() > index) {
		auto itr = this->m_results.begin();
		for (size_t i = 0; i < index; i++)
			itr++;
		return &(*itr);
	}
	return nullptr;
}

DISCO2_API::VisInfo* DRunManager::getVisinfo(size_t index) {
	if (this->m_results.size() > index) {
		auto itr = this->m_results.begin();
		for (size_t i = 0; i < index; i++)
			itr++;
		return &itr->visInfo;
	}
	return nullptr;
}

DISCO2_API::I_RecordsHandler* DRunManager::getRec(size_t index) {
	if (this->m_results.size() > index) {
		auto itr = this->m_results.begin();
		for (size_t i = 0; i < index; i++)
			itr++;
		return itr->rec;
	}
	return nullptr;
}

size_t DRunManager::addRec(DISCO2_API::I_RecordsHandler* rec,
		std::unordered_map<int, std::unordered_map<int, std::pair<std::vector<int64_t>, std::vector<int64_t>>>>&& sigRec,
		DISCO2_API::VisInfo&& visInfo) {
	this->m_results.emplace_back(RunRec{ rec, std::move(visInfo), std::move(sigRec) });
	return (this->m_results.size() - 1);
}

void DRunManager::removeAndDeleteRec(size_t index) {
	if (this->m_results.size() > index) {
		auto itr = this->m_results.begin();
		for (size_t i = 0; i < index; i++)
			itr++;
		delete itr->rec;
		this->m_results.erase(itr);
	}
}

