#pragma once

#include <list>
#include <unordered_map>
#include <vector>
#include <utility>

#include <VisInfo.h>

namespace DISCO2_API {
class I_RecordsHandler;
}

struct RunRec {
	DISCO2_API::I_RecordsHandler* rec;
	DISCO2_API::VisInfo visInfo;
	std::unordered_map<int, std::unordered_map<int, std::pair<std::vector<int64_t>, std::vector<int64_t>>>> sigRec;
};

class DRunManager {
private:

	std::list<RunRec> m_results;

public:

	// --- --- --- --- --- Constructor Destructor --- --- --- --- ---

	DRunManager();

	~DRunManager();

	// --- --- --- --- --- Getters --- --- --- --- ---

	size_t size();

	RunRec* getRunRec(size_t index);

	DISCO2_API::VisInfo* getVisinfo(size_t index);

	DISCO2_API::I_RecordsHandler* getRec(size_t index);

	// --- --- --- --- --- Mod Functions --- --- --- --- ---

	size_t addRec(DISCO2_API::I_RecordsHandler* rec, 
		std::unordered_map<int, std::unordered_map<int, std::pair<std::vector<int64_t>, std::vector<int64_t>>>>&& sigRec,
		DISCO2_API::VisInfo&& visInfo = {});

	void removeAndDeleteRec(size_t index);

};

