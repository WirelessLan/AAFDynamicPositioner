#include "PositionData.h"

#include <fstream>

#include "Positioners.h"
#include "Utils.h"

namespace PositionData {
	std::vector<Data> LoadPositionData(const std::string& a_position, bool a_isPlayerScene) {
		std::string posPath = a_isPlayerScene ?
			fmt::format("Data\\F4SE\\Plugins\\{}\\Player\\{}.txt", Version::PROJECT, a_position) : fmt::format("Data\\F4SE\\Plugins\\{}\\{}.txt", Version::PROJECT, a_position);
		std::ifstream posFile(posPath);

		std::vector<Data> result;

		if (!posFile.is_open())
			return result;

		std::string line;
		std::string indexStr, offX, offY, offZ;
		while (std::getline(posFile, line)) {
			Utils::Trim(line);
			if (line.empty() || line[0] == '#')
				continue;

			std::uint32_t index = 0;

			indexStr = Utils::GetNextData(line, index, '|');
			if (indexStr.empty()) {
				logger::error("Cannot read the position index: {}", line);
				continue;
			}

			offX = Utils::GetNextData(line, index, ',');
			if (offX.empty()) {
				logger::error("Cannot read the offsetX: {}", line);
				continue;
			}

			offY = Utils::GetNextData(line, index, ',');
			if (offY.empty()) {
				logger::error("Cannot read the offsetY: {}", line);
				continue;
			}

			offZ = Utils::GetNextData(line, index, 0);
			if (offZ.empty()) {
				logger::error("Cannot read the offsetZ: {}", line);
				continue;
			}

			result.push_back({ stoul(indexStr), RE::NiPoint3(stof(offX), stof(offY), stof(offZ)) });
		}

		return result;
	}

	bool SavePositionData(const std::string& a_position, const std::vector<std::uint32_t>& a_actors, bool a_isPlayerScene) {
		std::string posPath = a_isPlayerScene ?
			fmt::format("Data\\F4SE\\Plugins\\{}\\Player\\{}.txt", Version::PROJECT, a_position) : fmt::format("Data\\F4SE\\Plugins\\{}\\{}.txt", Version::PROJECT, a_position);
		std::ofstream posFile(posPath);

		if (!posFile.is_open())
			return false;

		for (auto formId : a_actors) {
			Positioners::ActorData* actorData = Positioners::GetActorDataByFormID(formId);
			if (!actorData)
				return false;

			posFile << actorData->PositionIndex << "|" << actorData->Offset.x << "," << actorData->Offset.y << "," << actorData->Offset.z << std::endl;
		}

		return true;
	}
}
