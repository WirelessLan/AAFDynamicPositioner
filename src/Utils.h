#pragma once

namespace Utils {
	void Trim(std::string& a_str);
	std::string GetNextData(const std::string& a_line, std::uint32_t& a_index, char a_delimeter);
	RE::TESForm* GetFormFromIdentifier(std::string_view a_pluginName, std::uint32_t a_formID);
	RE::TESForm* GetFormFromIdentifier(std::string_view a_pluginName, std::string_view a_formID);
	float GetActualScale(RE::TESObjectREFR* a_refr);
	void SetRefScale(RE::TESObjectREFR* a_refr, float a_scale);
	void ModPos(RE::TESObjectREFR* a_refr, char a_axis, float a_value);
	bool HasSpell(RE::Actor* a_actor, RE::SpellItem* a_spell);
	void AddSpell(RE::Actor* a_actor, RE::SpellItem* a_spell);
	void RemoveSpell(RE::Actor* a_actor, RE::SpellItem* a_spell);
}
