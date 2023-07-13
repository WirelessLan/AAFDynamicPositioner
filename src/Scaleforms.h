#pragma once

namespace Scaleforms {
	void RegisterMenu();
	void LoadLocalizations();
	void RegisterFunctions(RE::Scaleform::GFx::Movie* a_view, RE::Scaleform::GFx::Value* a_f4se_root);
	void OpenMenu(RE::NiPoint3& a_offset);
	void UpdateMenu(RE::NiPoint3& a_offset);
	void CloseMenu();
	bool IsMenuOpen();
}
