#include "Papyrus.h"

#include "DirectionalMovementHandler.h"
#include "Settings.h"

namespace Papyrus
{
	void TrueDirectionalMovement_MCM::OnConfigClose(ConsoleRE::TESQuest*)
	{
		Settings::ReadSettings();
	}

	bool TrueDirectionalMovement_MCM::Register(ConsoleRE::BSScript::IVirtualMachine* a_vm)
	{
		a_vm->BindNativeMethod(new ConsoleRE::BSScript::NativeFunction<false, decltype(OnConfigClose), ConsoleRE::TESQuest, void>("OnConfigClose", "TrueDirectionalMovement_MCM", OnConfigClose));

		xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "Registered TrueDirectionalMovement_MCM class");
		return true;
	}

    bool TrueDirectionalMovement::GetDirectionalMovementState(ConsoleRE::StaticFunctionTag*)
	{
	    return DirectionalMovementHandler::GetSingleton()->IsFreeCamera();
	}

    bool TrueDirectionalMovement::GetTargetLockState(ConsoleRE::StaticFunctionTag*)
	{
		return DirectionalMovementHandler::GetSingleton()->HasTargetLocked();
	}

    ConsoleRE::Actor* TrueDirectionalMovement::GetCurrentTarget(ConsoleRE::StaticFunctionTag*)
	{
		if (const auto currentTarget = DirectionalMovementHandler::GetSingleton()->GetTarget()) {
			return currentTarget.get().get();
		}

		return nullptr;
	}

    void TrueDirectionalMovement::ToggleDisableDirectionalMovement(ConsoleRE::StaticFunctionTag*, ConsoleRE::BSFixedString a_modName, bool a_bDisable)
	{
		if (a_modName.empty()) {
		    return;
		}

		DirectionalMovementHandler::GetSingleton()->PapyrusDisableDirectionalMovement(a_modName, a_bDisable);
	}

    void TrueDirectionalMovement::ToggleDisableHeadtracking(ConsoleRE::StaticFunctionTag*, ConsoleRE::BSFixedString a_modName, bool a_bDisable)
	{
		if (a_modName.empty()) {
			return;
		}

		DirectionalMovementHandler::GetSingleton()->PapyrusDisableHeadtracking(a_modName, a_bDisable);
	}

    bool TrueDirectionalMovement::Register(ConsoleRE::BSScript::IVirtualMachine* a_vm)
	{
		a_vm->BindNativeMethod(new ConsoleRE::BSScript::NativeFunction<false, decltype(GetDirectionalMovementState),		ConsoleRE::StaticFunctionTag, bool>									("GetDirectionalMovementState",			"TrueDirectionalMovement", GetDirectionalMovementState));
		a_vm->BindNativeMethod(new ConsoleRE::BSScript::NativeFunction<false, decltype(GetTargetLockState),					ConsoleRE::StaticFunctionTag, bool>									("GetTargetLockState",					"TrueDirectionalMovement", GetTargetLockState));
		a_vm->BindNativeMethod(new ConsoleRE::BSScript::NativeFunction<false, decltype(GetCurrentTarget),					ConsoleRE::StaticFunctionTag, ConsoleRE::Actor*>					("GetCurrentTarget",					"TrueDirectionalMovement", GetCurrentTarget));
		a_vm->BindNativeMethod(new ConsoleRE::BSScript::NativeFunction<false, decltype(ToggleDisableDirectionalMovement),	ConsoleRE::StaticFunctionTag, void, ConsoleRE::BSFixedString, bool>	("ToggleDisableDirectionalMovement",	"TrueDirectionalMovement", ToggleDisableDirectionalMovement));
		a_vm->BindNativeMethod(new ConsoleRE::BSScript::NativeFunction<false, decltype(ToggleDisableHeadtracking),			ConsoleRE::StaticFunctionTag, void, ConsoleRE::BSFixedString, bool>	("ToggleDisableHeadtracking",			"TrueDirectionalMovement", ToggleDisableHeadtracking));

		xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "Registered TrueDirectionalMovement class");

		return true;
	}

    void Register()
	{
		auto papyrus = API::GetPapyrusInterface();
		papyrus->RegisterPapyrusFunctions(TrueDirectionalMovement_MCM::Register);
		papyrus->RegisterPapyrusFunctions(TrueDirectionalMovement::Register);

		xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "Registered papyrus functions");
	}
}
