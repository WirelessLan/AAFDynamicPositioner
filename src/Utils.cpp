#include "Utils.h"

namespace Utils {
	void Trim(std::string& a_str) {
		a_str.erase(a_str.begin(), std::find_if(a_str.begin(), a_str.end(), [](int ch) {
			return !std::isspace(ch);
			}));
		a_str.erase(std::find_if(a_str.rbegin(), a_str.rend(), [](int ch) {
			return !std::isspace(ch);
			}).base(), a_str.end());
	}

	std::uint8_t GetNextChar(const std::string& a_line, std::uint32_t& a_index) {
		if (a_index < a_line.length())
			return a_line[a_index++];

		return 0xFF;
	}

	std::string GetNextData(const std::string& a_line, std::uint32_t& a_index, char a_delimeter) {
		std::uint8_t ch;
		std::string retVal = "";

		while ((ch = GetNextChar(a_line, a_index)) != 0xFF) {
			if (ch == '#') {
				if (a_index > 0) a_index--;
				break;
			}

			if (a_delimeter != 0 && ch == a_delimeter)
				break;

			retVal += static_cast<char>(ch);
		}

		Trim(retVal);
		return retVal;
	}

	std::uint32_t ParseFormID(std::string_view a_formID) {
		std::uint32_t retID = 0;
		try {
			retID = std::stoul(std::string(a_formID), nullptr, 16) & 0xFFFFFF;
		}
		catch (...) {}
		return retID;
	}

	RE::TESForm* GetFormFromIdentifier(std::string_view a_pluginName, std::uint32_t a_formID) {
		RE::TESDataHandler* g_dataHandler = RE::TESDataHandler::GetSingleton();
		if (!g_dataHandler)
			return nullptr;

		return g_dataHandler->LookupForm(a_formID, a_pluginName);
	}

	RE::TESForm* GetFormFromIdentifier(std::string_view a_pluginName, std::string_view a_formID) {
		std::uint32_t formID = ParseFormID(a_formID);
		return GetFormFromIdentifier(a_pluginName, formID);
	}

	float GetActualScale(RE::TESObjectREFR* a_refr) {
		if (!a_refr)
			return 0.0f;

		using func_t = float(*)(RE::TESObjectREFR*);
		REL::Relocation<func_t> func{ REL::ID(911188) };
		float actualScale = func(a_refr);

		RE::NiAVObject* skeletonNode = a_refr->Get3D(false);
		if (!skeletonNode)
			return actualScale;

		RE::NiAVObject* comNode = skeletonNode->GetObjectByName("COM");
		if (!comNode)
			return actualScale;

		actualScale *= comNode->local.scale;

		RE::NiAVObject* nodePtr = comNode->parent;
		while (nodePtr && nodePtr != skeletonNode) {
			actualScale *= nodePtr->local.scale;
			nodePtr = nodePtr->parent;
		}

		return actualScale;
	}

	void SetRefScale(RE::TESObjectREFR* a_refr, float a_scale) {
		float currActualScale = GetActualScale(a_refr);
		if (currActualScale == a_scale)
			return;

		float baseScale = currActualScale * 100 / a_refr->refScale;
		float modifiedRefScale = std::round(a_scale / baseScale * 100) / 100;

		using func_t = void(*)(RE::TESObjectREFR*, float);
		REL::Relocation<func_t> func{ REL::ID(817930) };
		func(a_refr, modifiedRefScale);
	}

	void ModPos(RE::TESObjectREFR* a_refr, char a_axis, float a_value) {
		using func_t = void(*)(RE::TESObjectREFR*, char, float);
		REL::Relocation<func_t> func{ REL::ID(334873) };
		func(a_refr, a_axis, a_value);
	}

	bool HasSpell(RE::Actor* a_actor, RE::SpellItem* a_spell) {
		if (!a_actor || !a_spell)
			return false;

		using func_t = bool(*)(RE::Actor*, RE::SpellItem*);
		REL::Relocation<func_t> func{ REL::ID(850247) };
		return func(a_actor, a_spell);
	}

	void AddSpell(RE::Actor* a_actor, RE::SpellItem* a_spell) {
		if (!a_actor || !a_spell)
			return;

		using func_t = void(*)(RE::Actor*, RE::SpellItem*);
		REL::Relocation<func_t> func{ REL::ID(1433810) };
		func(a_actor, a_spell);
	}

	void RemoveSpell(RE::Actor* a_actor, RE::SpellItem* a_spell) {
		if (!a_actor || !a_spell)
			return;

		using func_t = void(*)(RE::Actor*, RE::SpellItem*);
		REL::Relocation<func_t> func{ REL::ID(1500183) };
		func(a_actor, a_spell);
	}
}
