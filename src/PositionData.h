#pragma once

namespace PositionData {
	struct Data {
		std::uint32_t index;
		RE::NiPoint3  offset;
	};

	std::vector<Data> LoadPositionData(const std::string& a_position, bool a_isPlayerScene);
	bool SavePositionData(const std::string& a_position, const std::vector<std::uint32_t>& a_actorList, bool a_isPlayerScene);
}
