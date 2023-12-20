#include "Settings.h"

//
#include "../../../OrbisUtil/include/FileSystem.h"

// #include <toml++/toml.h>
#include "../../../OrbisUtil/Third-Party/ToruNiina/toml.hpp"

#include "DirectionalMovementHandler.h"

void Settings::Initialize()
{
	xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "Initializing...");

	auto dataHandler = ConsoleRE::TESDataHandler::GetSingleton();
	if (dataHandler) 
	{
		//
		kywd_magicWard					= dataHandler->LookupForm<ConsoleRE::BGSKeyword>(0x1EA69, "Skyrim.esm");
		kywd_furnitureForces1stPerson	= dataHandler->LookupForm<ConsoleRE::BGSKeyword>(0xA56D7, "Skyrim.esm");
		kywd_furnitureForces3rdPerson	= dataHandler->LookupForm<ConsoleRE::BGSKeyword>(0xA56D8, "Skyrim.esm");

		//
		spel_targetLockSpell			= dataHandler->LookupForm<ConsoleRE::SpellItem>(0x805, "TrueDirectionalMovement.esp");
		glob_directionalMovement		= dataHandler->LookupForm<ConsoleRE::TESGlobal>(0x807, "TrueDirectionalMovement.esp");
		glob_targetLockHint				= dataHandler->LookupForm<ConsoleRE::TESGlobal>(0x808, "TrueDirectionalMovement.esp");
		glob_trueHUD					= dataHandler->LookupForm<ConsoleRE::TESGlobal>(0x810, "TrueDirectionalMovement.esp");
		glob_nemesisHeadtracking		= dataHandler->LookupForm<ConsoleRE::TESGlobal>(0x811, "TrueDirectionalMovement.esp");
		glob_nemesisMountedArchery		= dataHandler->LookupForm<ConsoleRE::TESGlobal>(0x812, "TrueDirectionalMovement.esp");
		glob_nemesisLeaning				= dataHandler->LookupForm<ConsoleRE::TESGlobal>(0x813, "TrueDirectionalMovement.esp");
	}

	xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "...success");
}

void Settings::ReadSettings()
{
	//
	constexpr auto ext		= ".toml";
	constexpr auto path		= "/app0/data/SKSE/Plugins/TrueDirectionalMovement/";
	constexpr auto basecfg	= "/app0/data/SKSE/Plugins/TrueDirectionalMovement/TrueDirectionalMovement_base.toml";

	//
	constexpr auto mcmPath				= "/app0/data/MCM/Settings/TrueDirectionalMovement.ini";
	constexpr auto defaultSettingsPath	= "/app0/data/MCM/Config/TrueDirectionalMovement/settings.ini";
	
	//
	auto dataHandler = ConsoleRE::TESDataHandler::GetSingleton();

#if CPP17
	const auto readToml = [&](std::filesystem::path path)
	{
		xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "  Reading %s...", path.string());
		try
		{
			const auto tbl = toml::parse_file(path.c_str());
			auto& arr = *tbl.get_as<toml::array>("TargetPoints");
			for (auto&& elem : arr)
			{
				auto& targetPointsTbl = *elem.as_table();
				auto formID = targetPointsTbl["BodyPartDataFormID"].value<uint32_t>();
				auto pluginName = targetPointsTbl["Plugin"].value<const char*>();
				auto boneNames = targetPointsTbl["BoneNames"].as_array();
				if (boneNames)
				{
					auto bodyPartData = dataHandler->LookupForm<ConsoleRE::BGSBodyPartData>(*formID, *pluginName);
					if (bodyPartData)
					{
						std::vector<std::string> bones;
						for (auto& boneName : *boneNames)
						{
							bones.push_back(*boneName.value<std::string>());
						}

						targetPoints.insert_or_assign(bodyPartData, bones);
					}
				}
			}
		}
		catch ([[maybe_unused]] const toml::parse_error& e)
		{
			util::report_and_fail("Failed to load settings. This might be an indication of your game being unstable, try installing SSE Engine Fixes."sv);
		}
		catch (const std::exception& e)
		{
			util::report_and_fail(e.what());
		}
		catch (...)
		{
			util::report_and_fail("unknown failure"sv);
		}

	};

	/xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "Reading .toml files...");

	auto baseToml = std::filesystem::path(basecfg);
	readToml(baseToml);

	if (std::filesystem::is_directory(path))
	{
		for (const auto& file : std::filesystem::directory_iterator(path))
		{
			// read all toml files in Data/SKSE/Plugins/TrueDirectionalMovement folder
			if (std::filesystem::is_regular_file(file) && file.path().extension() == ext)
			{
				auto filePath = file.path();
				if (filePath != basecfg)
				{
					readToml(filePath);
				}
			}
		}
	}
#else
	const auto readToml = [&](const char* path)
	{
		const auto tbl = toml::parse(path);
		const auto arr = toml::find<toml::array>(tbl, "TargetPoints");

		for (auto&& elem : arr)
		{
			auto targetPointsTbl = elem.as_table();

			//
			auto formID = targetPointsTbl["BodyPartDataFormID"].as_integer();
			auto pluginName = targetPointsTbl["Plugin"].as_string();
			auto boneNames = targetPointsTbl["BoneNames"].as_array();

			if (boneNames.size())
			{
				auto bodyPartData = dataHandler->LookupForm<ConsoleRE::BGSBodyPartData>(formID, pluginName.str.c_str());
				if (bodyPartData)
				{
					std::vector<std::string> bones;
					for (auto& boneName : boneNames)
					{
						auto value = boneName.as_string();
						
						//
						bones.push_back(value.str.c_str());
					}

					targetPoints[bodyPartData] = bones;
				}
			}
		}
	};

	readToml(basecfg);

	for (auto& file : xUtilty::FileSystem::GetDirectoryEntries(path, ext, true, xUtilty::FileSystem::Full, true))
	{
		if (file.filename != basecfg)
		{
			readToml(file.filename.c_str());
		}
	}

#endif

	xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "...success");

	const auto readMCM = [&](const char* path) 
	{
		CSimpleIniA mcm;
		mcm.SetUnicode();

		mcm.LoadFile(path);

		// Directional Movement related
		ReadUInt32Setting(mcm, "DirectionalMovement", "uDirectionalMovementSheathed", (uint32_t&)uDirectionalMovementSheathed);
		ReadUInt32Setting(mcm, "DirectionalMovement", "uDirectionalMovementDrawn", (uint32_t&)uDirectionalMovementDrawn);
		ReadUInt32Setting(mcm, "DirectionalMovement", "uDialogueMode", (uint32_t&)uDialogueMode);
		ReadFloatSetting(mcm, "DirectionalMovement", "fMeleeMagnetismAngle", fMeleeMagnetismAngle);
		ReadBoolSetting(mcm, "DirectionalMovement", "bFaceCrosshairWhileAttacking", bFaceCrosshairWhileAttacking);
		ReadBoolSetting(mcm, "DirectionalMovement", "bFaceCrosshairWhileShouting", bFaceCrosshairWhileShouting);
		ReadBoolSetting(mcm, "DirectionalMovement", "bFaceCrosshairWhileBlocking", bFaceCrosshairWhileBlocking);
		ReadBoolSetting(mcm, "DirectionalMovement", "bFaceCrosshairDuringAutoMove", bFaceCrosshairDuringAutoMove);
		ReadBoolSetting(mcm, "DirectionalMovement", "bStopOnDirectionChange", bStopOnDirectionChange);
		ReadUInt32Setting(mcm, "DirectionalMovement", "uAdjustCameraYawDuringMovement", (uint32_t&)uAdjustCameraYawDuringMovement);
		ReadFloatSetting(mcm, "DirectionalMovement", "fRunningRotationSpeedMult", fRunningRotationSpeedMult);
		ReadFloatSetting(mcm, "DirectionalMovement", "fSprintingRotationSpeedMult", fSprintingRotationSpeedMult);
		ReadFloatSetting(mcm, "DirectionalMovement", "fAttackStartRotationSpeedMult", fAttackStartRotationSpeedMult);
		ReadFloatSetting(mcm, "DirectionalMovement", "fAttackMidRotationSpeedMult", fAttackMidRotationSpeedMult);
		ReadFloatSetting(mcm, "DirectionalMovement", "fAttackEndRotationSpeedMult", fAttackEndRotationSpeedMult);
		ReadFloatSetting(mcm, "DirectionalMovement", "fAirRotationSpeedMult", fAirRotationSpeedMult);
		ReadFloatSetting(mcm, "DirectionalMovement", "fGlidingRotationSpeedMult", fGlidingRotationSpeedMult);
		ReadFloatSetting(mcm, "DirectionalMovement", "fWaterRotationSpeedMult", fWaterRotationSpeedMult);
		ReadFloatSetting(mcm, "DirectionalMovement", "fSwimmingRotationSpeedMult", fSwimmingRotationSpeedMult);
		ReadFloatSetting(mcm, "DirectionalMovement", "fFaceCrosshairRotationSpeedMultiplier", fFaceCrosshairRotationSpeedMultiplier);
		ReadBoolSetting(mcm, "DirectionalMovement", "bFaceCrosshairInstantly", bFaceCrosshairInstantly);
		ReadFloatSetting(mcm, "DirectionalMovement", "fCameraAutoAdjustDelay", fCameraAutoAdjustDelay);
		ReadFloatSetting(mcm, "DirectionalMovement", "fCameraAutoAdjustSpeedMult", fCameraAutoAdjustSpeedMult);
		ReadBoolSetting(mcm, "DirectionalMovement", "bIgnoreSlowTime", bIgnoreSlowTime);
		ReadBoolSetting(mcm, "DirectionalMovement", "bDisableAttackRotationMultipliersForTransformations", bDisableAttackRotationMultipliersForTransformations);
		ReadFloatSetting(mcm, "DirectionalMovement", "fSwimmingPitchSpeed", fSwimmingPitchSpeed);
		ReadFloatSetting(mcm, "DirectionalMovement", "fControllerBufferDepth", fControllerBufferDepth);

		// Leaning
		ReadBoolSetting(mcm, "Leaning", "bEnableLeaning", bEnableLeaning);
		ReadBoolSetting(mcm, "Leaning", "bEnableLeaningNPC", bEnableLeaningNPC);
		ReadFloatSetting(mcm, "Leaning", "fLeaningMult", fLeaningMult);
		ReadFloatSetting(mcm, "Leaning", "fLeaningSpeed", fLeaningSpeed);
		ReadFloatSetting(mcm, "Leaning", "fMaxLeaningStrength", fMaxLeaningStrength);

		// Headtracking
		ReadBoolSetting(mcm, "Headtracking", "bHeadtracking", bHeadtracking);
		ReadBoolSetting(mcm, "Headtracking", "bHeadtrackSpine", bHeadtrackSpine);
		ReadFloatSetting(mcm, "Headtracking", "fDialogueHeadtrackingDuration", fDialogueHeadtrackingDuration);
		ReadBoolSetting(mcm, "Headtracking", "bCameraHeadtracking", bCameraHeadtracking);
		ReadFloatSetting(mcm, "Headtracking", "fCameraHeadtrackingStrength", fCameraHeadtrackingStrength);
		ReadFloatSetting(mcm, "Headtracking", "fCameraHeadtrackingDuration", fCameraHeadtrackingDuration);
		ReadUInt32Setting(mcm, "Headtracking", "uCameraHeadtrackingMode", (uint32_t&)uCameraHeadtrackingMode);

		// Target Lock
		ReadBoolSetting(mcm, "TargetLock", "bAutoTargetNextOnDeath", bAutoTargetNextOnDeath);
		ReadBoolSetting(mcm, "TargetLock", "bTargetLockTestLOS", bTargetLockTestLOS);
		ReadBoolSetting(mcm, "TargetLock", "bTargetLockHostileActorsOnly", bTargetLockHostileActorsOnly);
		ReadBoolSetting(mcm, "TargetLock", "bTargetLockHideCrosshair", bTargetLockHideCrosshair);
		ReadFloatSetting(mcm, "TargetLock", "fTargetLockDistance", fTargetLockDistance);
		ReadFloatSetting(mcm, "TargetLock", "fTargetLockDistanceMultiplierSmall", fTargetLockDistanceMultiplierSmall);
		ReadFloatSetting(mcm, "TargetLock", "fTargetLockDistanceMultiplierLarge", fTargetLockDistanceMultiplierLarge);
		ReadFloatSetting(mcm, "TargetLock", "fTargetLockDistanceMultiplierExtraLarge", fTargetLockDistanceMultiplierExtraLarge);
		ReadFloatSetting(mcm, "TargetLock", "fTargetLockPitchAdjustSpeed", fTargetLockPitchAdjustSpeed);
		ReadFloatSetting(mcm, "TargetLock", "fTargetLockYawAdjustSpeed", fTargetLockYawAdjustSpeed);
		ReadFloatSetting(mcm, "TargetLock", "fTargetLockPitchOffsetStrength", fTargetLockPitchOffsetStrength);
		ReadUInt32Setting(mcm, "TargetLock", "uTargetLockArrowAimType", (uint32_t&)uTargetLockArrowAimType);
		ReadUInt32Setting(mcm, "TargetLock", "uTargetLockMissileAimType", (uint32_t&)uTargetLockMissileAimType);
		ReadBoolSetting(mcm, "TargetLock", "bTargetLockUsePOVSwitchKeyboard", bTargetLockUsePOVSwitchKeyboard);
		ReadBoolSetting(mcm, "TargetLock", "bTargetLockUsePOVSwitchGamepad", bTargetLockUsePOVSwitchGamepad);
		ReadFloatSetting(mcm, "TargetLock", "fTargetLockPOVHoldDuration", fTargetLockPOVHoldDuration);
		ReadBoolSetting(mcm, "TargetLock", "bTargetLockUseMouse", bTargetLockUseMouse);
		ReadUInt32Setting(mcm, "TargetLock", "uTargetLockMouseSensitivity", (uint32_t&)uTargetLockMouseSensitivity);
		ReadBoolSetting(mcm, "TargetLock", "bTargetLockUseScrollWheel", bTargetLockUseScrollWheel);
		ReadBoolSetting(mcm, "TargetLock", "bTargetLockUseRightThumbstick", bTargetLockUseRightThumbstick);
		ReadBoolSetting(mcm, "TargetLock", "bResetCameraWithTargetLock", bResetCameraWithTargetLock);
		ReadBoolSetting(mcm, "TargetLock", "bResetCameraPitch", bResetCameraPitch);

		// HUD
		ReadBoolSetting(mcm, "HUD", "bEnableTargetLockReticle", bEnableTargetLockReticle);
		ReadUInt32Setting(mcm, "HUD", "uReticleAnchor", (uint32_t&)uReticleAnchor);
		ReadUInt32Setting(mcm, "HUD", "uReticleStyle", (uint32_t&)uReticleStyle);
		ReadFloatSetting(mcm, "HUD", "fReticleScale", fReticleScale);
		ReadBoolSetting(mcm, "HUD", "bReticleUseHUDOpacity", bReticleUseHUDOpacity);
		ReadFloatSetting(mcm, "HUD", "fReticleOpacity", fReticleOpacity);

		// Misc
		ReadBoolSetting(mcm, "Misc", "bOverrideAcrobatics", bOverrideAcrobatics);
		ReadFloatSetting(mcm, "Misc", "fAcrobatics", fAcrobatics);
		ReadFloatSetting(mcm, "Misc", "fAcrobaticsGliding", fAcrobaticsGliding);

		// Controller
		ReadBoolSetting(mcm, "Controller", "bOverrideControllerDeadzone", bOverrideControllerDeadzone);
		ReadFloatSetting(mcm, "Controller", "fControllerRadialDeadzone", fControllerRadialDeadzone);
		ReadFloatSetting(mcm, "Controller", "fControllerAxialDeadzone", fControllerAxialDeadzone);
		ReadBoolSetting(mcm, "Controller", "bThumbstickBounceFix", bThumbstickBounceFix);

		// Keys
		ReadUInt32Setting(mcm, "Keys", "uTargetLockKey", uTargetLockKey);
		ReadUInt32Setting(mcm, "Keys", "uSwitchTargetLeftKey", uSwitchTargetLeftKey);
		ReadUInt32Setting(mcm, "Keys", "uSwitchTargetRightKey", uSwitchTargetRightKey);
	};

	xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "Reading MCM .ini...");

	readMCM(defaultSettingsPath);  // read the default ini first
	readMCM(mcmPath);

	xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "...success");
	
	DirectionalMovementHandler::GetSingleton()->OnSettingsUpdated();
}

void Settings::OnPostLoadGame()
{
	UpdateGlobals();
}

void Settings::UpdateGlobals()
{
	if (glob_trueHUD) 
	{
		glob_trueHUD->value = DirectionalMovementHandler::GetSingleton()->g_trueHUD != nullptr ? 1.f : 0.f;
	}

	if ((glob_nemesisHeadtracking && glob_nemesisHeadtracking->value == 0) ||
		(glob_nemesisMountedArchery && glob_nemesisMountedArchery->value == 0) ||
		(glob_nemesisLeaning && glob_nemesisLeaning->value == 0))
	{
		auto playerCharacter = ConsoleRE::PlayerCharacter::GetSingleton();
		ConsoleRE::BSTSmartPointer<ConsoleRE::BSAnimationGraphManager> animationGraphManagerPtr;
		playerCharacter->GetAnimationGraphManager(animationGraphManagerPtr);
		if (animationGraphManagerPtr) 
		{
			ConsoleRE::BShkbAnimationGraph* animationGraph = animationGraphManagerPtr->graphs[0].get();
			if (animationGraph) 
			{
				if (glob_nemesisHeadtracking && glob_nemesisHeadtracking->value == 0) 
				{
					bool bDummy;
					glob_nemesisHeadtracking->value = animationGraph->GetGraphVariableBool("tdmHeadtrackingSKSE", bDummy);
				}

				if (glob_nemesisMountedArchery && glob_nemesisMountedArchery->value == 0) 
				{
					bool bDummy;
					glob_nemesisMountedArchery->value = playerCharacter->GetGraphVariableBool("360HorseGen", bDummy);
				}

				if (glob_nemesisLeaning && glob_nemesisLeaning->value == 0) 
				{
					float dummy;
					glob_nemesisLeaning->value = animationGraph->GetGraphVariableFloat("TDM_VelocityX", dummy);
				}
			}
		}
	}
}

void Settings::ReadBoolSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, bool& a_setting)
{
	const char* bFound = nullptr;
	bFound = a_ini.GetValue(a_sectionName, a_settingName);
	if (bFound)
	{
		a_setting = a_ini.GetBoolValue(a_sectionName, a_settingName);
	}
}

void Settings::ReadFloatSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, float& a_setting)
{
	const char* bFound = nullptr;
	bFound = a_ini.GetValue(a_sectionName, a_settingName);
	if (bFound) 
	{
		a_setting = static_cast<float>(a_ini.GetDoubleValue(a_sectionName, a_settingName));
	}
}

void Settings::ReadUInt32Setting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, uint32_t& a_setting)
{
	const char* bFound = nullptr;
	bFound = a_ini.GetValue(a_sectionName, a_settingName);
	if (bFound) 
	{
		a_setting = static_cast<uint32_t>(a_ini.GetLongValue(a_sectionName, a_settingName));
	}
}
