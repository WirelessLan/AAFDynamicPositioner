#pragma once

namespace Inputs {
	enum ACTION_KEY : std::uint32_t {
		kActionKey_TAB = 0x09,
		kActionKey_ENTER = 0x0D,
		kActionKey_LEFT = 0x25,
		kActionKey_UP = 0x26,
		kActionKey_RIGHT = 0x27,
		kActionKey_DOWN = 0x28,
	};

	bool IsValidKeycode(std::uint32_t a_keyCode);
	std::uint32_t GetMouseKeycode(std::int32_t a_idCode);
	std::uint32_t GamepadMaskToKeycode(std::uint32_t a_keyMask);
	std::uint32_t ReplaceKeyCodeForMenu(std::uint32_t a_keyCode);
	std::uint32_t DirectionToKeyCode(RE::DIRECTION_VAL a_dir);

	void BlockPlayerControls(bool a_block);
	void EnableMenuControls(std::map<RE::BSInputEventUser*, bool>& a_menuVec, bool a_enabled);
	void SetInputEnableLayer();
	void ResetInputEnableLayer();
}
