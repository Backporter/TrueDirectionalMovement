#pragma once

namespace Papyrus
{
	class TrueDirectionalMovement_MCM
	{
	public:
		static void OnConfigClose(ConsoleRE::TESQuest*);

		static bool Register(ConsoleRE::BSScript::IVirtualMachine* a_vm);
	};

	class TrueDirectionalMovement
	{
	public:
		static bool GetDirectionalMovementState(ConsoleRE::StaticFunctionTag*);
		static bool GetTargetLockState(ConsoleRE::StaticFunctionTag*);
		static ConsoleRE::Actor* GetCurrentTarget(ConsoleRE::StaticFunctionTag*);
		static void ToggleDisableDirectionalMovement(ConsoleRE::StaticFunctionTag*, ConsoleRE::BSFixedString a_modName, bool a_bDisable);
		static void ToggleDisableHeadtracking(ConsoleRE::StaticFunctionTag*, ConsoleRE::BSFixedString a_modName, bool a_bDisable);

		static bool Register(ConsoleRE::BSScript::IVirtualMachine* a_vm);
	};

	void Register();
}
