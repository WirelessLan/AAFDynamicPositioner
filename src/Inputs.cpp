#include "Inputs.h"

#include <Windows.h>
#include <Xinput.h>

namespace Inputs {
	bool g_inputEnableLayerEnabled;
	std::uint32_t g_inputEnableLayerIndex;

	enum MACRO : std::uint32_t {
		// first 256 for keyboard, then 8 mouse buttons, then mouse wheel up, wheel down, then 16 gamepad buttons
		kMacro_KeyboardOffset = 0,		// not actually used, just for self-documentation
		kMacro_NumKeyboardKeys = 256,

		kMacro_MouseButtonOffset = kMacro_NumKeyboardKeys,	// 256
		kMacro_NumMouseButtons = 8,

		kMacro_MouseWheelOffset = kMacro_MouseButtonOffset + kMacro_NumMouseButtons,	// 264
		kMacro_MouseWheelDirections = 2,

		kMacro_GamepadOffset = kMacro_MouseWheelOffset + kMacro_MouseWheelDirections,	// 266
		kMacro_NumGamepadButtons = 16,

		kMaxMacros = kMacro_GamepadOffset + kMacro_NumGamepadButtons	// 282
	};

	enum GAMEPAD_OFFSET : std::uint32_t {
		kGamepadButtonOffset_DPAD_UP = MACRO::kMacro_GamepadOffset,	// 266
		kGamepadButtonOffset_DPAD_DOWN,
		kGamepadButtonOffset_DPAD_LEFT,
		kGamepadButtonOffset_DPAD_RIGHT,
		kGamepadButtonOffset_START,
		kGamepadButtonOffset_BACK,
		kGamepadButtonOffset_LEFT_THUMB,
		kGamepadButtonOffset_RIGHT_THUMB,
		kGamepadButtonOffset_LEFT_SHOULDER,
		kGamepadButtonOffset_RIGHT_SHOULDER,
		kGamepadButtonOffset_A,
		kGamepadButtonOffset_B,
		kGamepadButtonOffset_X,
		kGamepadButtonOffset_Y,
		kGamepadButtonOffset_LT,
		kGamepadButtonOffset_RT	// 281
	};

	class BSInputEnableManager {
	public:
		enum {
			kUserEvent_Movement = 1 << 0,
			kUserEvent_Looking = 1 << 1,
			kUserEvent_Activate = 1 << 2,
			kUserEvent_Menu = 1 << 3,
			kUserEvent_Console = 1 << 4,
			kUserEvent_POVChange = 1 << 5,
			kUserEvent_Fighting = 1 << 6,
			kUserEvent_Sneaking = 1 << 7,
			kUserEvent_MainFourMenu = 1 << 8,
			kUserEvent_WheelZoom = 1 << 9,
			kUserEvent_Jumping = 1 << 10
		};

		enum {
			kOtherEvent_JournalTabs = 1 << 0,
			kOtherEvent_Activation = 1 << 1,
			kOtherEvent_FastTravel = 1 << 2,
			kOtherEvent_POVChange = 1 << 3,
			kOtherEvent_VATS = 1 << 4,
			kOtherEvent_FAVORITES = 1 << 5,
			kOtherEvent_PipboyLight = 1 << 6,
			kOtherEvent_ZKey = 1 << 7,
			kOtherEvent_Running = 1 << 8
		};

		struct InputEnableLayerState {
			std::uint32_t index;
			std::uint32_t state;
		};

		static BSInputEnableManager* GetSingleton() {
			REL::Relocation<BSInputEnableManager**> InputEnableManager{ REL::ID(781703) };
			auto ptr = InputEnableManager.get();
			if (!ptr)
				return nullptr;
			return *ptr;
		}

		bool EnableUserEvent(std::uint32_t a_layerIdx, std::uint32_t a_flag, bool a_enable, std::uint32_t arg4) {
			using func_t = bool(*)(BSInputEnableManager*, std::uint32_t, std::uint32_t, bool, std::uint32_t);
			REL::Relocation<func_t> func{ REL::ID(1432984) };
			return func(this, a_layerIdx, a_flag, a_enable, arg4);
		}

		bool EnableOtherEvent(std::uint32_t a_layerIdx, std::uint32_t a_flag, bool a_enable, std::uint32_t arg4) {
			using func_t = bool(*)(BSInputEnableManager*, std::uint32_t, std::uint32_t, bool, std::uint32_t);
			REL::Relocation<func_t> func{ REL::ID(1419268) };
			return func(this, a_layerIdx, a_flag, a_enable, arg4);
		}

		void ResetInputEnableLayer(std::uint32_t a_layerIdx) {
			std::uint64_t flag = static_cast<std::uint64_t>(-1);
			EnableUserEvent(a_layerIdx, flag & 0xFFFFFFFF, true, 3);
			EnableOtherEvent(a_layerIdx, flag >> 32, true, 3);
		}

		std::uint64_t unk00[0x118 >> 3];						// 000
		std::uint64_t currentInputEnableMask;					// 118
		std::uint64_t unk120;									// 120
		RE::BSSpinLock inputEnableArrLock;						// 128
		RE::BSTArray<std::uint64_t>	inputEnableMaskArr;			// 130
		RE::BSTArray<InputEnableLayerState*> layerStateArr;		// 148
		RE::BSTArray<RE::BSFixedString> layerNameArr;			// 160
	};

	bool IsValidKeycode(std::uint32_t a_keyCode) {
		return a_keyCode < Inputs::kMaxMacros;
	}

	std::uint32_t GetMouseKeycode(std::int32_t a_idCode) {
		return kMacro_MouseButtonOffset + a_idCode;
	}

	std::uint32_t GamepadMaskToKeycode(std::uint32_t a_keyMask) {
		switch (a_keyMask) {
		case XINPUT_GAMEPAD_DPAD_UP:		return kGamepadButtonOffset_DPAD_UP;
		case XINPUT_GAMEPAD_DPAD_DOWN:		return kGamepadButtonOffset_DPAD_DOWN;
		case XINPUT_GAMEPAD_DPAD_LEFT:		return kGamepadButtonOffset_DPAD_LEFT;
		case XINPUT_GAMEPAD_DPAD_RIGHT:		return kGamepadButtonOffset_DPAD_RIGHT;
		case XINPUT_GAMEPAD_START:			return kGamepadButtonOffset_START;
		case XINPUT_GAMEPAD_BACK:			return kGamepadButtonOffset_BACK;
		case XINPUT_GAMEPAD_LEFT_THUMB:		return kGamepadButtonOffset_LEFT_THUMB;
		case XINPUT_GAMEPAD_RIGHT_THUMB:	return kGamepadButtonOffset_RIGHT_THUMB;
		case XINPUT_GAMEPAD_LEFT_SHOULDER:	return kGamepadButtonOffset_LEFT_SHOULDER;
		case XINPUT_GAMEPAD_RIGHT_SHOULDER: return kGamepadButtonOffset_RIGHT_SHOULDER;
		case XINPUT_GAMEPAD_A:				return kGamepadButtonOffset_A;
		case XINPUT_GAMEPAD_B:				return kGamepadButtonOffset_B;
		case XINPUT_GAMEPAD_X:				return kGamepadButtonOffset_X;
		case XINPUT_GAMEPAD_Y:				return kGamepadButtonOffset_Y;
		case 0x9:							return kGamepadButtonOffset_LT;
		case 0xA:							return kGamepadButtonOffset_RT;
		default:							return kMaxMacros; // Invalid
		}
	}

	std::uint32_t ReplaceKeyCodeForMenu(std::uint32_t a_keyCode) {
		switch (a_keyCode) {
		case kGamepadButtonOffset_DPAD_UP:
		case 0x57:	// W Key
			return ACTION_KEY::kActionKey_UP;
			break;

		case kGamepadButtonOffset_DPAD_DOWN:
		case 0x53:	// S Key
			return ACTION_KEY::kActionKey_DOWN;
			break;

		case kGamepadButtonOffset_DPAD_LEFT:
		case 0x41:	// A Key
			return ACTION_KEY::kActionKey_LEFT;
			break;

		case kGamepadButtonOffset_DPAD_RIGHT:
		case 0x44:	// D Key
			return ACTION_KEY::kActionKey_RIGHT;
			break;

		case kGamepadButtonOffset_A:
		case 0x45:	// E Key
			return ACTION_KEY::kActionKey_ENTER;
			break;

		case kGamepadButtonOffset_B:
			return ACTION_KEY::kActionKey_TAB;
			break;
		}

		return a_keyCode;
	}

	std::uint32_t DirectionToKeyCode(RE::DIRECTION_VAL a_dir) {
		switch (a_dir) {
		case RE::DIRECTION_VAL::kUp:
			return ACTION_KEY::kActionKey_UP;
			break;

		case RE::DIRECTION_VAL::kDown:
			return ACTION_KEY::kActionKey_DOWN;
			break;

		case RE::DIRECTION_VAL::kLeft:
			return ACTION_KEY::kActionKey_LEFT;
			break;

		case RE::DIRECTION_VAL::kRight:
			return ACTION_KEY::kActionKey_RIGHT;
			break;
		}

		return 0xFF;
	}

	bool SetInputEnableLayer(std::uint32_t a_userEventFlag, std::uint32_t a_otherEventFlag) {
		BSInputEnableManager* g_inputEnableManager = BSInputEnableManager::GetSingleton();
		if (!g_inputEnableManager)
			return false;

		g_inputEnableManager->inputEnableArrLock.lock();

		g_inputEnableLayerIndex = 0xFFFFFFFF;
		for (auto layerState : g_inputEnableManager->layerStateArr) {
			if (layerState->state != 1)
				continue;

			g_inputEnableLayerIndex = layerState->index;
			break;
		}

		if (g_inputEnableLayerIndex == 0xFFFFFFFF) {
			g_inputEnableManager->inputEnableArrLock.unlock();
			return false;
		}

		g_inputEnableManager->layerStateArr[g_inputEnableLayerIndex]->state = 2;
		g_inputEnableManager->layerNameArr[g_inputEnableLayerIndex] = "AAF Dynamic Positioner Menu Input Layer";
		g_inputEnableManager->EnableUserEvent(g_inputEnableLayerIndex, a_userEventFlag, false, 3);
		g_inputEnableManager->EnableOtherEvent(g_inputEnableLayerIndex, a_otherEventFlag, false, 3);

		g_inputEnableManager->inputEnableArrLock.unlock();

		return true;
	}

	void SetInputEnableLayer() {
		if (g_inputEnableLayerEnabled)
			return;

		g_inputEnableLayerEnabled = true;
		std::uint32_t userEventFlag = BSInputEnableManager::kUserEvent_Menu | BSInputEnableManager::kUserEvent_Fighting;
		std::uint32_t otherEventFlag = BSInputEnableManager::kOtherEvent_POVChange;
		SetInputEnableLayer(userEventFlag, otherEventFlag);
	}

	bool ResetInputEnableLayer(std::uint32_t a_layerIdx) {
		if (a_layerIdx == 0xFFFFFFFF)
			return false;

		BSInputEnableManager* g_inputEnableManager = BSInputEnableManager::GetSingleton();
		if (!g_inputEnableManager)
			return false;

		g_inputEnableManager->inputEnableArrLock.lock();

		std::uint64_t flag = 0xFFFFFFFFFFFFFFFF;
		g_inputEnableManager->EnableUserEvent(a_layerIdx, flag & 0xFFFFFFFF, true, 3);
		g_inputEnableManager->EnableOtherEvent(a_layerIdx, flag >> 32, true, 3);
		g_inputEnableManager->layerStateArr[a_layerIdx]->state = 1;
		g_inputEnableManager->layerNameArr[a_layerIdx] = "";

		g_inputEnableManager->inputEnableArrLock.unlock();

		return true;
	}

	void ResetInputEnableLayer() {
		if (!g_inputEnableLayerEnabled)
			return;

		g_inputEnableLayerEnabled = false;
		ResetInputEnableLayer(g_inputEnableLayerIndex);
	}

	void BlockPlayerControls(bool a_block) {
		RE::PlayerControls* g_pc = RE::PlayerControls::GetSingleton();
		if (!g_pc)
			return;

		g_pc->blockPlayerInput = a_block;
	}

	void EnableMenuControls(std::map<RE::BSInputEventUser*, bool>& a_menuVec, bool a_enabled) {
		RE::MenuControls* g_menuControls = RE::MenuControls::GetSingleton();
		if (!g_menuControls)
			return;

		if (!a_enabled)
			a_menuVec.clear();

		// 0 ~ 7 is reserved
		for (std::uint32_t ii = 8; ii < g_menuControls->handlers.size(); ii++) {
			if (!a_enabled) {
				a_menuVec.insert(std::make_pair(g_menuControls->handlers[ii], g_menuControls->handlers[ii]->inputEventHandlingEnabled));
				g_menuControls->handlers[ii]->inputEventHandlingEnabled = a_enabled;
			}
			else {
				auto it = a_menuVec.find(g_menuControls->handlers[ii]);
				if (it != a_menuVec.end())
					g_menuControls->handlers[ii]->inputEventHandlingEnabled = it->second;
			}
		}
	}
}
