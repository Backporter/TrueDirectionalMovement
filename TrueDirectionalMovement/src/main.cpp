#include "../../../OrbisUtil/include/RelocationManager.h"
#include "../../../OrbisUtil/include/Logger.h"
#include "../../../OrbisUtil/include/FileSystem.h"

#include "DirectionalMovementHandler.h"
#include "Events.h"
#include "Hooks.h"
#include "ModAPI.h"
#include "Papyrus.h"
#include "Settings.h"
#include "SmoothCamAPI.h"
#include "TrueHUDAPI.h"
#include "Raycast.h"

extern "C" __declspec (dllexport) int module_start(size_t argc, const void* argv) { xUtilty::RelocationManager(); return 0; }
extern "C" __declspec (dllexport) int module_stop(size_t argc, const void* argv) { return 0; }

void MessageHandler(Interface::MessagingInterface::Message* a_msg)
{
	switch (a_msg->type) 
	{
	case Interface::MessagingInterface::kDataLoad:
		DirectionalMovementHandler::RequestAPIs();
		Events::SinkEventHandlers();
		Settings::Initialize();
		Settings::ReadSettings();		
		DirectionalMovementHandler::GetSingleton()->InitCameraModsCompatibility();
		DirectionalMovementHandler::GetSingleton()->Initialize();
		break;
	case Interface::MessagingInterface::kPluginsLoaded:
		DirectionalMovementHandler::RequestAPIs();
		break;
	case Interface::MessagingInterface::kPluginsLoaded2:
		DirectionalMovementHandler::RequestAPIs();
		break;
	case Interface::MessagingInterface::kLoading:
		DirectionalMovementHandler::GetSingleton()->OnPreLoadGame();
		break;
	case Interface::MessagingInterface::kLoaded:
	case Interface::MessagingInterface::kNewGame:
		DirectionalMovementHandler::RequestAPIs();
		Settings::OnPostLoadGame();
		DirectionalMovementHandler::Register();
		break;
	}
}

namespace
{
	void InitializeLog()
	{
		xUtilty::Log::GetSingleton(0)->OpenRelitive(xUtilty::FileSystem::Download, "OSEL/Plugins/TrueDirectionalMovement/TrueDirectionalMovement.log");
	}
}

extern "C" DLLEXPORT bool Query(const Interface::QueryInterface* a_skse, PluginInfo* a_info)
{
	a_info->SetPluginName("TrueDirectionalMovement");
	a_info->SetPluginVersion(1);

	return true;
}

extern "C" DLLEXPORT bool Load(const Interface::QueryInterface* a_skse)
{
	InitializeLog();

	API::Init(a_skse);
	API::AllocTrampoline(1 << 9);

	auto messaging = API::GetMessagingInterface();
	if (!messaging->RegisterPluginCallback("SELF", MessageHandler)) 
	{
		return false;
	}

	Hooks::Install();
	Papyrus::Register();

	return true;
}

extern "C" DLLEXPORT bool Revert()
{
	return true;
}

extern "C" DLLEXPORT void* RequestPluginAPI(const TDM_API::InterfaceVersion a_interfaceVersion)
{
	auto api = Messaging::TDMInterface::GetSingleton();

	xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "TrueDirectionalMovement::RequestPluginAPI called, InterfaceVersion %d", static_cast<uint8_t>(a_interfaceVersion));

	switch (a_interfaceVersion) 
	{
	case TDM_API::InterfaceVersion::V1:
		[[fallthrough]];
	case TDM_API::InterfaceVersion::V2:
		xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "TrueDirectionalMovement::RequestPluginAPI returned the API singleton");
		return static_cast<void*>(api);
	}

	xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "TrueDirectionalMovement::RequestPluginAPI requested the wrong interface version");
	return nullptr;
}
