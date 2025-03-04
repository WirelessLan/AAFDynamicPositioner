#include <Windows.h>

#include "Positioners.h"
#include "Scaleforms.h"

std::string GetINIOption(const char* a_section, const char* a_key) {
	static const std::string configPath = fmt::format("Data\\MCM\\Settings\\{}.ini", Version::PROJECT);
	char resultBuf[256]{};
	GetPrivateProfileStringA(a_section, a_key, "", resultBuf, sizeof(resultBuf), configPath.c_str());
	return resultBuf;
}

void ReadINI() {
	std::string value;

	value = GetINIOption("Settings", "bSeparatePlayerOffset");
	if (!value.empty()) {
		try {
			Positioners::g_separatePlayerOffset = std::stoul(value);
		}
		catch (...) {}
	}
	logger::info("bSeparatePlayerOffset: {}", Positioners::g_separatePlayerOffset);

	value = GetINIOption("Settings", "bUnifyAAFDoppelgangerScale");
	if (!value.empty()) {
		try {
			Positioners::g_unifyAAFDoppelgangerScale = std::stoul(value);
		}
		catch (...) {}
	}
	logger::info("bUnifyAAFDoppelgangerScale: {}", Positioners::g_unifyAAFDoppelgangerScale);

	value = GetINIOption("Settings", "iPlayerPositionerType");
	if (!value.empty()) {
		try {
			Positioners::g_playerPositionerType = std::stoul(value);
		}
		catch (...) {}
	}
	logger::info("iPlayerPositionerType: {}", Positioners::g_playerPositionerType);

	value = GetINIOption("Settings", "iNPCPositionerType");
	if (!value.empty()) {
		try {
			Positioners::g_npcPositionerType = std::stoul(value);
		}
		catch (...) {}
	}
	logger::info("iNPCPositionerType: {}", Positioners::g_npcPositionerType);
}

void OnF4SEMessage(F4SE::MessagingInterface::Message* a_msg) {
	switch (a_msg->type) {
	case F4SE::MessagingInterface::kGameLoaded:
		Scaleforms::RegisterMenu();
		Scaleforms::LoadLocalizations();
		break;

	case F4SE::MessagingInterface::kNewGame:
	case F4SE::MessagingInterface::kPreLoadGame:
		Positioners::ResetPositioner();
		break;
	}
}

bool RegisterPapyrusFunctions(RE::BSScript::IVirtualMachine* a_vm) {
	Positioners::Install(a_vm);
	return true;
}

bool RegisterScaleforms(RE::Scaleform::GFx::Movie* a_view, RE::Scaleform::GFx::Value* a_value) {
	Scaleforms::RegisterFunctions(a_view, a_value);
	return true;
}

extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Query(const F4SE::QueryInterface * a_f4se, F4SE::PluginInfo * a_info) {
#ifndef NDEBUG
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
	auto path = logger::log_directory();
	if (!path) {
		return false;
	}

	*path /= fmt::format("{}.log", Version::PROJECT);
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

	auto log = std::make_shared<spdlog::logger>("Global Log"s, std::move(sink));

#ifndef NDEBUG
	log->set_level(spdlog::level::trace);
#else
	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::trace);
#endif

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%^%l%$] %v"s);

	logger::info("{} v{}", Version::PROJECT, Version::NAME);

	a_info->infoVersion = F4SE::PluginInfo::kVersion;
	a_info->name = Version::PROJECT.data();
	a_info->version = Version::MAJOR;

	if (a_f4se->IsEditor()) {
		logger::critical("loaded in editor");
		return false;
	}

	const auto ver = a_f4se->RuntimeVersion();
	if (ver < F4SE::RUNTIME_1_10_162) {
		logger::critical("unsupported runtime v{}", ver.string());
		return false;
	}

	return true;
}

extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Load(const F4SE::LoadInterface * a_f4se) {
	F4SE::Init(a_f4se);

	ReadINI();

	const F4SE::MessagingInterface* message = F4SE::GetMessagingInterface();
	if (message) {
		message->RegisterListener(OnF4SEMessage);
	}

	const F4SE::PapyrusInterface* papyrus = F4SE::GetPapyrusInterface();
	if (papyrus) {
		papyrus->Register(RegisterPapyrusFunctions);
	}

	const F4SE::ScaleformInterface* scaleform = F4SE::GetScaleformInterface();
	if (scaleform) {
		scaleform->Register(Version::PROJECT, RegisterScaleforms);
	}

	return true;
}
