#pragma once

// variables
static void** g_142EC5C60						= (void**)REL::Relocation<uintptr_t>(0x30CA390).address();						// 2EC5C60, 2F603B0

//
static uintptr_t g_worldToCamMatrix				= REL::Relocation<uintptr_t>(0x319B050).address();								// 2F4C910, 2FE75F0
static uintptr_t g_highActorCuller				= REL::Relocation<uintptr_t>(0x313E748).address();								// 2F26F80, 2FC1A50

//
static ConsoleRE::NiRect<float>* g_viewPort		= (ConsoleRE::NiRect<float>*)REL::Relocation<uintptr_t>(0x319B028).address();	// 2F4DED0, 2FE8B98

//
static bool*  g_bDampenPlayerControls			= (bool*)REL::Relocation<uintptr_t>(0x318F838).address();						// 1DF1498, 1E858A8
static float* g_fHUDOpacity						= (float*)REL::Relocation<uintptr_t>(0x31A03F8).address();						// 1DF58F8, 1E89D38
static float* g_fControllerBufferDepth			= (float*)REL::Relocation<uintptr_t>(0x318F808).address();						// 1DF1468, 1E85878
static float* g_fSprintStopThreshold			= (float*)REL::Relocation<uintptr_t>(0x31902D0).address();						// 1DF1680, 1E85A90
static float* g_fLThumbDeadzone					= (float*)REL::Relocation<uintptr_t>(0x373A820).address();						// 1E05DD0, 1E9A278
static float* g_fLThumbDeadzoneMax				= (float*)REL::Relocation<uintptr_t>(0x373A868).address();						// 1E05E18, 1E9A2C0
static float* g_fActivatePickLength				= (float*)REL::Relocation<uintptr_t>(0x3100988).address();						// 1DD7E88, 1E6BF68
static float* g_f3PArrowTiltUpAngle				= (float*)REL::Relocation<uintptr_t>(0x30CA168).address();						// 1DB52C8, 1E49338
static float* g_fFreeRotationSpeed				= (float*)REL::Relocation<uintptr_t>(0x3198800).address();						// 1DF3820, 1E87BC8
static float* g_fVanityModeMinDist				= (float*)REL::Relocation<uintptr_t>(0x3198788).address();						// 1DF37A8, 1E87B50
static float* g_fVanityModeMaxDist				= (float*)REL::Relocation<uintptr_t>(0x31987B8).address();						// 1DF37D8, 1E87B80

//
static float* g_deltaTime						= (float*)REL::Relocation<uintptr_t>(0x31C7478).address();						// 2F6B948, 30064C8
static float* g_deltaTimeRealTime				= (float*)REL::Relocation<uintptr_t>(0x31C747C).address();						// 2F6B94C, 30064CC
static float* g_DurationOfApplicationRunTimeMS	= (float*)REL::Relocation<uintptr_t>(0x31C7480).address();						// 2F6B950, 30064D0

static float* g_worldScale						= (float*)REL::Relocation<uintptr_t>(0x1782934).address();						// 154064C, 1637AA0
static float* g_worldScaleInverse				= (float*)REL::Relocation<uintptr_t>(0x172EB64).address();						// 1536BA0, 162DF48



// functions
typedef ConsoleRE::NiAVObject*(*tNiAVObject_LookupBoneNodeByName)(ConsoleRE::NiAVObject*, const ConsoleRE::BSFixedString&, bool);
typedef ConsoleRE::BGSMovementType*(*tGetMovementTypeFromString)(const char**);

typedef int64_t(*tBShkbAnimationGraph_GetCurrentMovementTypeName)(ConsoleRE::BShkbAnimationGraph*, ConsoleRE::BSFixedString*);

typedef bool(*tBSInputDeviceManager_IsUsingGamepad)(ConsoleRE::BSInputDeviceManager*);
typedef bool(* tActor_IsSyncSprintState)(ConsoleRE::Actor*);
typedef bool(* tActor_CanSprint_CheckCharacterControllerValues)(ConsoleRE::Actor*);

typedef void(*tPlayerControls_CalledWhenSprintStateChanges)(ConsoleRE::PlayerControls*, int32_t, int32_t);
typedef void(*tNiQuaternion_SomeRotationManipulation)(ConsoleRE::NiQuaternion&, float, float, float);
typedef void(*tPlayerControls_ApplyLookSensitivitySettings)(ConsoleRE::PlayerControls*, ConsoleRE::NiPoint2*);
typedef float(*tTESObjectREFR_GetSubmergeLevel)(ConsoleRE::TESObjectREFR*, float, ConsoleRE::TESObjectCELL*);

typedef void*(*tCastRay)(ConsoleRE::hkpWorld*, ConsoleRE::hkpWorldRayCastInput&, ConsoleRE::hkpRayHitCollector&);
typedef void*(*tLinearCast)(ConsoleRE::hkpWorld*, const ConsoleRE::hkpCollidable*, const ConsoleRE::hkpLinearCastInput*, ConsoleRE::hkpCdPointCollector*, ConsoleRE::hkpCdPointCollector* );
typedef void(tPlayerCamera_SetCameraState)(ConsoleRE::PlayerCamera*, ConsoleRE::CameraState);
typedef void(tAIProcess_ClearHeadTrackTarget)(ConsoleRE::AIProcess*);

typedef float(*tGetPlayerTimeMult)(void*);

//
static REL::Relocation<tGetMovementTypeFromString>						GetMovementTypeFromString						(0x2E2AB0); // { RELOCATION_ID(23268, 23727) };
static REL::Relocation<tBSInputDeviceManager_IsUsingGamepad>			BSInputDeviceManager_IsUsingGamepad				(0xE36DE0); // { RELOCATION_ID(67320, 68622) };
static REL::Relocation<tBShkbAnimationGraph_GetCurrentMovementTypeName> BShkbAnimationGraph_GetCurrentMovementTypeName	(0xBA1200); // { RELOCATION_ID(62654, 63599) };
static REL::Relocation<tActor_IsSyncSprintState>						Actor_IsSyncSprintState							(0x6DDA40); // { RELOCATION_ID(36924, 37949) };
static REL::Relocation<tPlayerControls_CalledWhenSprintStateChanges>	PlayerControls_CalledWhenSprintStateChanges		(0x85FA70); // { RELOCATION_ID(41271, 42350) };
static REL::Relocation<tNiQuaternion_SomeRotationManipulation>			NiQuaternion_SomeRotationManipulation			(0xE92220); // { RELOCATION_ID(69466, 70843) };
static REL::Relocation<tPlayerControls_ApplyLookSensitivitySettings>	PlayerControls_ApplyLookSensitivitySettings		(0x85F2F0); // { RELOCATION_ID(41275, 42354) };
static REL::Relocation<tNiAVObject_LookupBoneNodeByName>				NiAVObject_LookupBoneNodeByName					(0xF8B860); // { RELOCATION_ID(74481, 76207) };
static REL::Relocation<tCastRay>										CastRay											(0xCC2E90); // { RELOCATION_ID(60552, 61400) };
static REL::Relocation<tLinearCast>										LinearCast										(0xCC3E00); // { RELOCATION_ID(60554, 61402) };
static REL::Relocation<tActor_CanSprint_CheckCharacterControllerValues> Actor_CanSprint_CheckCharacterControllerValues	(0x6D6990); // { RELOCATION_ID(36260, 37244) };
static REL::Relocation<tTESObjectREFR_GetSubmergeLevel>					TESObjectREFR_GetSubmergeLevel					(0x6D7DD0); // { RELOCATION_ID(36452, 37448) };
static REL::Relocation<tGetPlayerTimeMult>								GetPlayerTimeMult								(0x8C8FF0); // { RELOCATION_ID(43104, 44301) };  // 759420, 7873C0
static REL::Relocation<tAIProcess_ClearHeadTrackTarget>					AIProcess_ClearHeadTrackTarget					(0x7B8270); // { RELOCATION_ID(38852, 39889) };  // 67D190, 6A4900
static REL::Relocation<tPlayerCamera_SetCameraState>					PlayerCamera_SetCameraState						(0x9D13E0); // { RELOCATION_ID(49947, 50880) };  // 84EA70, 87A930
