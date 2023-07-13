#pragma once

namespace Positioners {
	class ExtraRefrPath : public RE::BSExtraData {
	public:
		RE::NiPoint3	startPos;	// 18
		RE::NiPoint3	startTan;	// 24
		RE::NiPoint3	startEuler;	// 30
		RE::NiPoint3	goalPos;	// 3C
		RE::NiPoint3	goalTan;	// 48
		RE::NiPoint3	goalEuler;	// 54
	};

	struct ActorData {
		std::uint32_t	FormID;
		RE::Actor*		Actor;
		std::uint64_t	SceneID;
		std::uint32_t	PositionIndex;
		ExtraRefrPath*	ExtraRefrPath;
		RE::NiPoint3	OriginalPosition;
		RE::NiPoint3	Offset;
	};
	
	extern bool g_separatePlayerOffset;
	extern bool g_unifyAAFDoppelgangerScale;
	extern std::uint32_t g_playerPositionerType;
	extern std::uint32_t g_npcPositionerType;

	void Install(RE::BSScript::IVirtualMachine* a_vm);
	ActorData* GetActorDataByFormID(std::uint32_t a_formID);
	void SetOffset(const std::string& a_axis, float a_offset);
	void ClearOffset();
	void ResetPositioner();
}
