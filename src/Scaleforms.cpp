#include "Scaleforms.h"

#include <fstream>

#include "Positioners.h"
#include "Inputs.h"
#include "Utils.h"

namespace Scaleforms {
	const std::string_view g_menuName = "AAFDynamicPositionerMenu";
	RE::NiPoint3 g_offset;
	std::map<RE::BSInputEventUser*, bool> g_menuEnableMap;

	class PositionerMenu : public RE::IMenu {
	public:
		PositionerMenu() : RE::IMenu() {
			Instance = this;
			Instance->menuFlags = (RE::UI_MENU_FLAGS)0;
			Instance->UpdateFlag(RE::UI_MENU_FLAGS::kUsesCursor, true);
			Instance->UpdateFlag(RE::UI_MENU_FLAGS::kTopmostRenderedMenu, true);
			Instance->inputEventHandlingEnabled = false;

			RE::BSScaleformManager* g_scaleformManager = RE::BSScaleformManager::GetSingleton();
			if (g_scaleformManager)
				g_scaleformManager->LoadMovie(*Instance, uiMovie, g_menuName.data(), "root1");
		}

		~PositionerMenu() {
			Instance = nullptr;
		}

		bool ShouldHandleEvent(const RE::InputEvent*) override {
			return true;
		}

		void OnButtonEvent(const RE::ButtonEvent* a_inputEvent) override {
			std::uint32_t keyCode;

			// Mouse
			if (a_inputEvent->device == RE::INPUT_DEVICE::kMouse)
				keyCode = Inputs::GetMouseKeycode(a_inputEvent->idCode);
			// Gamepad
			else if (a_inputEvent->device == RE::INPUT_DEVICE::kGamepad)
				keyCode = Inputs::GamepadMaskToKeycode(a_inputEvent->idCode);
			// Keyboard
			else
				keyCode = a_inputEvent->idCode;

			// Valid scancode?
			if (!Inputs::IsValidKeycode(keyCode))
				return;

			keyCode = Inputs::ReplaceKeyCodeForMenu(keyCode);

			bool isDown = a_inputEvent->value == 1.0f;

			sendKeyEvent(keyCode, isDown);
		}

		void OnThumbstickEvent(const RE::ThumbstickEvent* a_inputEvent) override {
			if (a_inputEvent->idCode != RE::ThumbstickEvent::THUMBSTICK_ID::kLeft)
				return;

			uint32_t prevKeyCode = Inputs::DirectionToKeyCode(a_inputEvent->prevDir);
			uint32_t currKeyCode = Inputs::DirectionToKeyCode(a_inputEvent->currDir);
			if (currKeyCode != 0xFF) {
				sendKeyEvent(currKeyCode, true);
			}
			else {
				if (currKeyCode == prevKeyCode) {
					sendKeyEvent(0x25, false);
					sendKeyEvent(0x26, false);
					sendKeyEvent(0x27, false);
					sendKeyEvent(0x28, false);
				}
				else {
					sendKeyEvent(prevKeyCode, false);
				}
			}
		}

		static PositionerMenu* GetSingleton() {
			return Instance;
		}

	private:
		void sendKeyEvent(uint32_t a_keyCode, bool a_isDown) {
			PositionerMenu* g_menuInst = PositionerMenu::GetSingleton();
			if (!g_menuInst)
				return;

			RE::Scaleform::GFx::ASMovieRootBase* movieRoot = g_menuInst->uiMovie->asMovieRoot.get();
			RE::Scaleform::GFx::Value root;
			if (!movieRoot || !movieRoot->GetVariable(&root, "root")) {
				logger::critical("SendKeyEvent: Couldn't get a root");
				return;
			}

			RE::Scaleform::GFx::Value params[2];
			params[0] = a_keyCode;
			params[1] = a_isDown;

			root.Invoke("ProcessKeyEvent", nullptr, params, 2);
		}

		static inline PositionerMenu* Instance = nullptr;
	};

	class Localizations {
	public:
		Localizations() {}

		static Localizations* GetSingleton() {
			static Localizations loc;
			return &loc;
		}

		std::string_view lang;
		std::unordered_map<std::string, std::string> translationsMap;
	};

	class ThrowHandler : public RE::Scaleform::GFx::FunctionHandler {
	public:
		virtual void Call(const Params& a_params) override {
			if (a_params.argCount > 0)
				logger::critical(FMT_STRING("Throw Message: {}"), a_params.args[0].GetString());
			CloseMenu();
		}
	};

	class CloseHandler : public RE::Scaleform::GFx::FunctionHandler {
	public:
		virtual void Call(const Params&) override {
			CloseMenu();
		}
	};

	class GetInitializationDataHandler : public RE::Scaleform::GFx::FunctionHandler {
	public:
		virtual void Call(const Params& a_params) override {
			RE::IMenu* g_menu = PositionerMenu::GetSingleton();
			g_menu->inputEventHandlingEnabled = true;

			RE::Scaleform::GFx::ASMovieRootBase* movieRoot = a_params.movie->asMovieRoot.get();
			if (!movieRoot) {
				logger::critical("GetInitializationDataHandler: Couldn't get a movieRoot!");
				return;
			}

			Localizations* g_loc = Localizations::GetSingleton();
			if (!g_loc) {
				logger::critical("GetInitializationDataHandler: Couldn't get the localization data!");
				return;
			}

			movieRoot->CreateObject(a_params.retVal);

			RE::Scaleform::GFx::Value locVal;
			movieRoot->CreateObject(&locVal);
			for (const auto& transPair : g_loc->translationsMap)
				locVal.SetMember(transPair.first, RE::Scaleform::GFx::Value(transPair.second.c_str()));

			a_params.retVal->SetMember("Language", g_loc->lang.data());
			a_params.retVal->SetMember("Localizations", locVal);
		}
	};

	class InitializationCompleteHandler : public RE::Scaleform::GFx::FunctionHandler {
	public:
		virtual void Call(const Params& a_params) override {
			RE::Scaleform::GFx::ASMovieRootBase* movieRoot = a_params.movie->asMovieRoot.get();
			RE::Scaleform::GFx::Value root;
			if (!movieRoot || !movieRoot->GetVariable(&root, "root")) {
				logger::critical("InitializationCompleteHandler: Couldn't get a root!");
				return;
			}

			RE::Scaleform::GFx::Value offset[3];
			offset[0] = g_offset.x;
			offset[1] = g_offset.y;
			offset[2] = g_offset.z;

			root.Invoke("ShowMenu", nullptr, offset, 3);
		}
	};

	class UpdateSettingsHandler : public RE::Scaleform::GFx::FunctionHandler {
	public:
		virtual void Call(const Params& a_params) override {
			if (a_params.argCount == 0 || a_params.args[0].GetType() != RE::Scaleform::GFx::Value::ValueType::kString)
				return;

			if (a_params.argCount == 2) {
				if (strcmp(a_params.args[0].GetString(), "bSeparatePlayerOffset") == 0)
					Positioners::g_separatePlayerOffset = a_params.args[1].GetBoolean();
				else if (strcmp(a_params.args[0].GetString(), "bUnifyAAFDoppelgangerScale") == 0)
					Positioners::g_unifyAAFDoppelgangerScale = a_params.args[1].GetBoolean();
				else if (strcmp(a_params.args[0].GetString(), "iPlayerPositionerType") == 0)
					Positioners::g_playerPositionerType = a_params.args[1].GetInt();
				else if (strcmp(a_params.args[0].GetString(), "iNPCPositionerType") == 0)
					Positioners::g_npcPositionerType = a_params.args[1].GetInt();
			}
		}
	};

	class SetPositionHandler : public RE::Scaleform::GFx::FunctionHandler {
	public:
		virtual void Call(const Params& a_params) override {
			std::string axis = a_params.args[0].GetString();
			float value = static_cast<float>(a_params.args[1].GetNumber());

			Positioners::SetOffset(axis, value);
		}
	};

	class ClearPositionHandler : public RE::Scaleform::GFx::FunctionHandler {
	public:
		virtual void Call(const Params&) override {
			Positioners::ClearOffset();
		}
	};

	void RegisterMenu() {
		RE::UI* g_ui = RE::UI::GetSingleton();
		if (g_ui) {
			g_ui->RegisterMenu(g_menuName.data(), [](const RE::UIMessage&) -> RE::IMenu* {
				PositionerMenu* menu = PositionerMenu::GetSingleton();
				if (!menu)
					menu = new PositionerMenu();
				return menu;
			});

			logger::info("Menu Registered");
		}
	}

	void LoadLocalizations() {
		Localizations* loc = Localizations::GetSingleton();

		loc->lang = "en";	// Default sLanguage
		RE::INISettingCollection* g_iniSettings = RE::INISettingCollection::GetSingleton();
		if (g_iniSettings) {
			for (RE::Setting* set : g_iniSettings->settings) {
				if (set->GetKey() == "sLanguage:General"sv) {
					loc->lang = set->GetString();
					break;
				}
			}
		}

		std::string transPath = fmt::format(FMT_STRING("Data\\Interface\\Translations\\{}_{}.txt"), g_menuName, loc->lang);
		std::ifstream transFile(transPath);
		if (!transFile.is_open()) {
			bool noTrans = false;

			if (loc->lang != "en") {
				logger::warn(FMT_STRING("Cannot open the translation file: {}"), transPath);

				transPath = fmt::format(FMT_STRING("Data\\Interface\\Translations\\{}_en.txt"), g_menuName);
				transFile.clear();
				transFile.open(transPath);
				if (!transFile.is_open())
					noTrans = true;
			}
			else
				noTrans = true;

			if (noTrans) {
				logger::warn(FMT_STRING("Cannot find the translation file: {}"), transPath);
				return;
			}
		}

		std::string line;
		std::string name, value;
		while (std::getline(transFile, line)) {
			Utils::Trim(line);
			if (line.empty() || line.starts_with('#'))
				continue;

			std::uint32_t index = 0;

			name = Utils::GetNextData(line, index, '\t');
			if (name.empty()) {
				logger::warn(FMT_STRING("Cannot read the name: {}"), line);
				continue;
			}

			value = Utils::GetNextData(line, index, 0);
			if (value.empty()) {
				logger::warn(FMT_STRING("Cannot read the value: {}"), line);
				continue;
			}

			loc->translationsMap.insert(std::make_pair(name, value));
		}
	}

	void RegisterFunction(RE::Scaleform::GFx::Movie* a_view, RE::Scaleform::GFx::Value* a_f4se_root, RE::Scaleform::GFx::FunctionHandler* a_handler, F4SE::stl::zstring a_name) {
		RE::Scaleform::GFx::Value fn;
		a_view->CreateFunction(&fn, a_handler);
		a_f4se_root->SetMember(a_name, fn);
	}

	void RegisterFunctions(RE::Scaleform::GFx::Movie* a_view, RE::Scaleform::GFx::Value* a_f4se_root) {
		RegisterFunction(a_view, a_f4se_root, new ThrowHandler(), "Throw"sv);
		RegisterFunction(a_view, a_f4se_root, new CloseHandler(), "Close"sv);
		RegisterFunction(a_view, a_f4se_root, new GetInitializationDataHandler(), "GetInitializationData"sv);
		RegisterFunction(a_view, a_f4se_root, new InitializationCompleteHandler(), "InitializationComplete"sv);
		RegisterFunction(a_view, a_f4se_root, new UpdateSettingsHandler(), "UpdateSettings"sv);
		RegisterFunction(a_view, a_f4se_root, new SetPositionHandler(), "SetPosition"sv);
		RegisterFunction(a_view, a_f4se_root, new ClearPositionHandler(), "ClearPosition"sv);
	}

	void OpenMenu(RE::NiPoint3& a_offset) {
		g_offset = a_offset;

		Inputs::BlockPlayerControls(true);
		Inputs::EnableMenuControls(g_menuEnableMap, false);
		Inputs::SetInputEnableLayer();

		RE::UIMessageQueue* g_uiMessageQueue = RE::UIMessageQueue::GetSingleton();
		if (g_uiMessageQueue)
			g_uiMessageQueue->AddMessage(g_menuName, RE::UI_MESSAGE_TYPE::kShow);
	}

	void UpdateMenu(RE::NiPoint3& a_offset) {
		if (!IsMenuOpen())
			return;

		g_offset = a_offset;

		PositionerMenu* g_menuInst = PositionerMenu::GetSingleton();
		if (!g_menuInst)
			return;

		RE::Scaleform::GFx::ASMovieRootBase* movieRoot = g_menuInst->uiMovie->asMovieRoot.get();
		RE::Scaleform::GFx::Value root;
		if (!movieRoot || !movieRoot->GetVariable(&root, "root")) {
			logger::critical("UpdateMenu: Couldn't get a root");
			return;
		}

		RE::Scaleform::GFx::Value offset[3];
		offset[0] = g_offset.x;
		offset[1] = g_offset.y;
		offset[2] = g_offset.z;

		root.Invoke("UpdateOffset", nullptr, offset, 3);
	}

	void CloseMenu() {
		Inputs::BlockPlayerControls(false);
		Inputs::EnableMenuControls(g_menuEnableMap, true);
		Inputs::ResetInputEnableLayer();

		RE::UIMessageQueue* g_uiMessageQueue = RE::UIMessageQueue::GetSingleton();
		if (g_uiMessageQueue)
			g_uiMessageQueue->AddMessage(g_menuName, RE::UI_MESSAGE_TYPE::kHide);
	}

	bool IsMenuOpen() {
		RE::UI* g_ui = RE::UI::GetSingleton();
		if (!g_ui)
			return false;
		
		return g_ui->GetMenuOpen(g_menuName);
	}
}
