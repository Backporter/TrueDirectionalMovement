#pragma once

namespace Hooks
{
	class MovementHook
	{
	public:
		static void Hook()
		{
			REL::Relocation<uintptr_t> MovementHandlerVtbl{ ConsoleRE::VTABLE_MovementHandler[0] };

			_ProcessThumbstick = MovementHandlerVtbl.write_vfunc(0x5, ProcessThumbstick); // 0x2
			_ProcessButton = MovementHandlerVtbl.write_vfunc(0x3, ProcessButton); // 0x4
		}

	private:
		static void ProcessThumbstick(ConsoleRE::MovementHandler* a_this, ConsoleRE::ThumbstickEvent* a_event, ConsoleRE::PlayerControlsData* a_data);
		static void ProcessButton(ConsoleRE::MovementHandler* a_this, ConsoleRE::ButtonEvent* a_event, ConsoleRE::PlayerControlsData* a_data);

		static inline REL::Relocation<decltype(ProcessThumbstick)> _ProcessThumbstick;
		static inline REL::Relocation<decltype(ProcessButton)> _ProcessButton;
	};

	class GamepadHook
	{
	public:
		static void Hook()
		{
			auto& trampoline = API::GetTrampoline();

			REL::Relocation<uintptr_t> hook(0xE3D620);										 // C1AB40, C40F80
			_ProcessInput = trampoline.write_call<5>(hook.address() + 0x5F3, ProcessInput);  // C1AC7F, C410BF
		}

	private:
		static void ProcessInput(ConsoleRE::BSOrbisGamepadDevice* a_this, int32_t a_rawX, int32_t a_rawY, float a_deadzoneMin, float a_deadzoneMax, float& a_outX, float& a_outY);

		static inline REL::Relocation<decltype(ProcessInput)> _ProcessInput;
	};

	class LookHook
	{
	public:
		static void Hook()
		{
			REL::Relocation<uintptr_t> LookHandlerVtbl{ ConsoleRE::VTABLE_LookHandler[0] };

			_ProcessThumbstick = LookHandlerVtbl.write_vfunc(0x5, ProcessThumbstick);
			_ProcessMouseMove = LookHandlerVtbl.write_vfunc(0x4, ProcessMouseMove);
		}

	private:
		static void ProcessThumbstick(ConsoleRE::LookHandler* a_this, ConsoleRE::ThumbstickEvent* a_event, ConsoleRE::PlayerControlsData* a_data);
		static void ProcessMouseMove(ConsoleRE::LookHandler* a_this, ConsoleRE::MouseMoveEvent* a_event, ConsoleRE::PlayerControlsData* a_data);

		static inline REL::Relocation<decltype(ProcessThumbstick)> _ProcessThumbstick;
		static inline REL::Relocation<decltype(ProcessMouseMove)> _ProcessMouseMove;
	};

	class TogglePOVHook
	{
	public:
		static void Hook()
		{
			REL::Relocation<uintptr_t> TogglePOVHandlerVtbl{ ConsoleRE::VTABLE_TogglePOVHandler[0] };

			_ProcessButton = TogglePOVHandlerVtbl.write_vfunc(0x3, ProcessButton);
		}

	private:
		static void ProcessButton(ConsoleRE::TogglePOVHandler* a_this, ConsoleRE::ButtonEvent* a_event, ConsoleRE::PlayerControlsData* a_data);

		static inline REL::Relocation<decltype(ProcessButton)> _ProcessButton;
	};

	class FirstPersonStateHook
	{
	public:
		static void Hook()
		{
			REL::Relocation<uintptr_t> FirstPersonStateVtbl{ ConsoleRE::VTABLE_FirstPersonState[0] };
			_OnEnterState = FirstPersonStateVtbl.write_vfunc(0x2, OnEnterState);
			_OnExitState = FirstPersonStateVtbl.write_vfunc(0x3, OnExitState);
			
			REL::Relocation<uintptr_t> PlayerInputHandlerVtbl{ ConsoleRE::VTABLE_FirstPersonState[0] };
			_ProcessButton = PlayerInputHandlerVtbl.write_vfunc(0xB, ProcessButton);
		}

	private:
		static void OnEnterState(ConsoleRE::FirstPersonState* a_this);
		static void OnExitState(ConsoleRE::FirstPersonState* a_this);
		static void ProcessButton(ConsoleRE::FirstPersonState* a_this, ConsoleRE::ButtonEvent* a_event, ConsoleRE::PlayerControlsData* a_data);
		
		static inline REL::Relocation<decltype(OnEnterState)> _OnEnterState;
		static inline REL::Relocation<decltype(OnExitState)> _OnExitState;
		static inline REL::Relocation<decltype(ProcessButton)> _ProcessButton;
	};

	class ThirdPersonStateHook
	{
	public:
		static void Hook()
		{
			REL::Relocation<uintptr_t> ThirdPersonStateVtbl{ ConsoleRE::VTABLE_ThirdPersonState[0] };
			_OnEnterState = ThirdPersonStateVtbl.write_vfunc(0x2, OnEnterState);
			_OnExitState = ThirdPersonStateVtbl.write_vfunc(0x3, OnExitState);
			_SetFreeRotationMode = ThirdPersonStateVtbl.write_vfunc(0xE, SetFreeRotationMode);
			
			REL::Relocation<uintptr_t> PlayerInputHandlerVtbl{ ConsoleRE::VTABLE_ThirdPersonState[0] };
			_ProcessButton = PlayerInputHandlerVtbl.write_vfunc(0x10, ProcessButton);
		}
	private:		
		static void OnEnterState(ConsoleRE::ThirdPersonState* a_this);
		static void OnExitState(ConsoleRE::ThirdPersonState* a_this);
		static void SetFreeRotationMode(ConsoleRE::ThirdPersonState* a_this, bool a_weaponSheathed);
		static void ProcessButton(ConsoleRE::ThirdPersonState* a_this, ConsoleRE::ButtonEvent* a_event, ConsoleRE::PlayerControlsData* a_data);

		static inline REL::Relocation<decltype(OnEnterState)> _OnEnterState;
		static inline REL::Relocation<decltype(OnExitState)> _OnExitState;
		static inline REL::Relocation<decltype(SetFreeRotationMode)> _SetFreeRotationMode;
		static inline REL::Relocation<decltype(ProcessButton)> _ProcessButton;
	};

	class HorseCameraStateHook
	{
	public:
		static void Hook()
		{
			REL::Relocation<uintptr_t> HorseCameraStateVtbl{ ConsoleRE::VTABLE_HorseCameraState[0] };
			_OnEnterState = HorseCameraStateVtbl.write_vfunc(0x2, OnEnterState);
			_OnExitState = HorseCameraStateVtbl.write_vfunc(0x3, OnExitState);
			_UpdateRotation = HorseCameraStateVtbl.write_vfunc(0x11, UpdateRotation);
			_HandleLookInput = HorseCameraStateVtbl.write_vfunc(0x12, HandleLookInput);
			
			REL::Relocation<uintptr_t> PlayerInputHandlerVtbl{ ConsoleRE::VTABLE_HorseCameraState[1] };
			_ProcessButton = PlayerInputHandlerVtbl.write_vfunc(0x3, ProcessButton);
		}

	private:
		static void OnEnterState(ConsoleRE::HorseCameraState* a_this);
		static void OnExitState(ConsoleRE::HorseCameraState* a_this);
		static void UpdateRotation(ConsoleRE::HorseCameraState* a_this);
		static void HandleLookInput(ConsoleRE::HorseCameraState* a_this, const ConsoleRE::NiPoint2& a_input); 
		static void ProcessButton(ConsoleRE::HorseCameraState* a_this, ConsoleRE::ButtonEvent* a_event, ConsoleRE::PlayerControlsData* a_data);

		static inline REL::Relocation<decltype(OnEnterState)> _OnEnterState;
		static inline REL::Relocation<decltype(OnExitState)> _OnExitState;
		static inline REL::Relocation<decltype(UpdateRotation)> _UpdateRotation;
		static inline REL::Relocation<decltype(HandleLookInput)> _HandleLookInput;
		static inline REL::Relocation<decltype(ProcessButton)> _ProcessButton;
	};

	class TweenMenuCameraStateHook
	{
	public:
		static void Hook()
		{
			REL::Relocation<uintptr_t> TweenMenuCameraStateVtbl{ ConsoleRE::VTABLE_TweenMenuCameraState[0] };
			_OnEnterState = TweenMenuCameraStateVtbl.write_vfunc(0x2, OnEnterState);
			_OnExitState = TweenMenuCameraStateVtbl.write_vfunc(0x3, OnExitState);
		}

	private:
		static void OnEnterState(ConsoleRE::TESCameraState* a_this);
		static void OnExitState(ConsoleRE::TESCameraState* a_this);

		static inline REL::Relocation<decltype(OnEnterState)> _OnEnterState;
		static inline REL::Relocation<decltype(OnExitState)> _OnExitState;
	};

	class VATSCameraStateHook
	{
	public:
		static void Hook()
		{
			REL::Relocation<uintptr_t> VATSCameraStateVtbl{ ConsoleRE::VTABLE_VATSCameraState[0] };
			_OnExitState = VATSCameraStateVtbl.write_vfunc(0x3, OnExitState);
		}

	private:
		static void OnExitState(ConsoleRE::TESCameraState* a_this);

		static inline REL::Relocation<decltype(OnExitState)> _OnExitState;
	};

	//
	class PlayerCameraTransitionStateHook
	{
	public:
		static void Hook()
		{
			REL::Relocation<uintptr_t> PlayerCameraTransitionStateVtbl{ ConsoleRE::VTABLE_PlayerCameraTransitionState[0] };
			_OnEnterState = PlayerCameraTransitionStateVtbl.write_vfunc(0x2, OnEnterState);
		}

	private:
		static void OnEnterState(ConsoleRE::PlayerCameraTransitionState* a_this);

		static inline REL::Relocation<decltype(OnEnterState)> _OnEnterState;
	};

	class MovementHandlerAgentPlayerControlsHook
	{
	public:
		static void Hook()
		{
			REL::Relocation<uintptr_t> MovementHandlerAgentPlayerControls_IMovementHandlerAgentVtbl{ ConsoleRE::VTABLE_MovementHandlerAgentPlayerControls[0] };  // 166ABA0
			_Func1 = MovementHandlerAgentPlayerControls_IMovementHandlerAgentVtbl.write_vfunc(0x3, Func1);
		}
	private:
		static void Func1(void* a1, void* a2);

		static inline REL::Relocation<decltype(Func1)> _Func1;
	};

	class ProjectileHook
	{
	public:
		static void Hook()
		{
			REL::Relocation<uintptr_t> ProjectileVtbl{ ConsoleRE::VTABLE_Projectile[0] };					// 167C888
			REL::Relocation<uintptr_t> ArrowProjectileVtbl{ ConsoleRE::VTABLE_ArrowProjectile[0] };			// 1676318
			REL::Relocation<uintptr_t> MissileProjectileVtbl{ ConsoleRE::VTABLE_MissileProjectile[0] };		// 167AE78
			REL::Relocation<uintptr_t> BeamProjectileVtbl{ ConsoleRE::VTABLE_BeamProjectile[0] };			// 1677660
			
			_GetLinearVelocityProjectile = ProjectileVtbl.write_vfunc(0x8E, GetLinearVelocityProjectile);
			_GetLinearVelocityArrow = ArrowProjectileVtbl.write_vfunc(0x8E, GetLinearVelocityArrow);
			_GetLinearVelocityMissile = MissileProjectileVtbl.write_vfunc(0x8E, GetLinearVelocityMissile);

			auto& trampoline = API::GetTrampoline();
			REL::Relocation<uintptr_t> hook(0x8B5BC0); // { RELOCATION_ID(43030, 44222) };  // 754820, 7821A0

			trampoline.write_call<6>(hook.address() + 0x59F /*RELOCATION_OFFSET(0x318, 0x6FF)*/, Func183);  // 754B24, 78289F // vfunc call
			_InitProjectile = trampoline.write_call<5>(hook.address() + 0x65E/*RELOCATION_OFFSET(0x3B8, 0x78A)*/, InitProjectile);
		}
	private:
		static void ProjectileAimSupport(ConsoleRE::Projectile* a_this);
		static void GetLinearVelocityProjectile(ConsoleRE::Projectile* a_this, ConsoleRE::NiPoint3& a_outVelocity);
		static void GetLinearVelocityArrow(ConsoleRE::Projectile* a_this, ConsoleRE::NiPoint3& a_outVelocity);
		static void GetLinearVelocityMissile(ConsoleRE::Projectile* a_this, ConsoleRE::NiPoint3& a_outVelocity);
		static bool Func183(ConsoleRE::Projectile* a_this);
		static void InitProjectile(ConsoleRE::Projectile* a_this);

		static inline REL::Relocation<decltype(GetLinearVelocityProjectile)> _GetLinearVelocityProjectile;
		static inline REL::Relocation<decltype(GetLinearVelocityArrow)> _GetLinearVelocityArrow;
		static inline REL::Relocation<decltype(GetLinearVelocityMissile)> _GetLinearVelocityMissile;
		static inline REL::Relocation<decltype(InitProjectile)> _InitProjectile;
	};

	class CharacterHook
	{
	public:
		static void Hook()
		{
			REL::Relocation<uintptr_t> CharacterVtbl{ ConsoleRE::VTABLE_Character[0] };
			_Update = CharacterVtbl.write_vfunc(0xB6, Update);
		}

	private:
		static void Update(ConsoleRE::Actor* a_this, float a_delta);

		static inline REL::Relocation<decltype(Update)> _Update;
	};

	class PlayerCharacterHook
	{
	public:
		static void Hook()
		{
			REL::Relocation<uintptr_t> PlayerCharacterVtbl{ ConsoleRE::VTABLE_PlayerCharacter[0] };					// 1665E0
			_UpdateAnimation = PlayerCharacterVtbl.write_vfunc(0x85, UpdateAnimation);
			_Update = PlayerCharacterVtbl.write_vfunc(0xB6, Update);
			_ProcessTracking = PlayerCharacterVtbl.write_vfunc(0x145, ProcessTracking);
			
			REL::Relocation<uintptr_t> PlayerCharacter_ActorStateVtbl{ ConsoleRE::VTABLE_PlayerCharacter[6] };  // 16640E8
			_GetAngle = PlayerCharacter_ActorStateVtbl.write_vfunc(0x5, GetAngle);

			auto& trampoline = API::GetTrampoline();
			REL::Relocation<uintptr_t> hook(0x7E1910); // { RELOCATION_ID(39375, 40447) };  // 69E580, 6C6440

			_UpdateSprintState = trampoline.write_call<5>(hook.address() + 0x19B4/*RELOCATION_OFFSET2(0xDAE, 0x140B, 0x140D)*/, UpdateSprintState);
		}

	private:
		static void UpdateAnimation(ConsoleRE::Actor* a_this, float a_delta);
		static void Update(ConsoleRE::Actor* a_this, float a_delta);
		static void ProcessTracking(ConsoleRE::Actor* a_this, float a_delta, ConsoleRE::NiAVObject* a_obj3D);
		static void GetAngle(ConsoleRE::ActorState* a_this, ConsoleRE::NiPoint3 &a_angle);
		static void UpdateSprintState(ConsoleRE::PlayerCharacter* a_this);

		static inline REL::Relocation<decltype(UpdateAnimation)> _UpdateAnimation;
		static inline REL::Relocation<decltype(Update)> _Update;
		static inline REL::Relocation<decltype(ProcessTracking)> _ProcessTracking;
		static inline REL::Relocation<decltype(GetAngle)> _GetAngle;
		static inline REL::Relocation<decltype(UpdateSprintState)> _UpdateSprintState;
	};

	class PlayerControlsHook
	{
	public:
		static void Hook()
		{
			REL::Relocation<uintptr_t> PlayerControlsVtbl{ ConsoleRE::VTABLE_PlayerControls[0] };  // 166E838
			_Handle = PlayerControlsVtbl.write_vfunc(0x2, Handle);

			auto& trampoline = API::GetTrampoline();			
			REL::Relocation<uintptr_t> hook(0x85E630); // { RELOCATION_ID(41288, 42338) };  // 706AF0, 72E720

			_CheckIsInSyncAnim = trampoline.write_call<5>(hook.address() + 0x107 /*RELOCATION_OFFSET(0xD8, 0x153)*/, CheckIsInSyncAnim);
			_Check2 = trampoline.write_call<5>(hook.address() + 0xC4 /*RELOCATION_OFFSET(0x99, 0x106)*/, Check2);
			_Check3 = trampoline.write_call<5>(hook.address() + 0xDC /*RELOCATION_OFFSET(0xB9, 0x12A)*/, Check3);
		}

	private:
		static bool Handle(ConsoleRE::PlayerControls* a_this, ConsoleRE::InputEvent** a_event);
		//static bool CanProcessControls(ConsoleRE::PlayerControls* a_this, ConsoleRE::InputEvent** a_eventPtr);
		static bool CheckIsInSyncAnim(void* a_a1, void* a_a2);
		static bool Check2(ConsoleRE::PlayerCharacter* a_this);
		static bool Check3(ConsoleRE::PlayerCharacter* a_this);
		static inline REL::Relocation<decltype(Handle)> _Handle;
		//static inline REL::Relocation<decltype(CanProcessControls)> _CanProcessControls;
		static inline REL::Relocation<decltype(CheckIsInSyncAnim)> _CheckIsInSyncAnim;
		static inline REL::Relocation<decltype(Check2)> _Check2;
		static inline REL::Relocation<decltype(Check3)> _Check3;

	};

	class AIProcess_SetRotationSpeedZHook
	{
	public:
		static void Hook()
		{
			auto& trampoline = API::GetTrampoline();
			REL::Relocation<uintptr_t> hook1(0x6DB4C0); // { RELOCATION_ID(36365, 37356) };  // 5D87F0, 5FD7E0
			REL::Relocation<uintptr_t> hook2(0x85F9E0); // { RELOCATION_ID(41293, 42373) };  // 707210, 731330

			_AIProcess_SetRotationSpeedZ1 = trampoline.write_call<5>(hook1.address() + 0x618 /* RELOCATION_OFFSET(0x356, 0x3EF) */, AIProcess_SetRotationSpeedZ1);
			_AIProcess_SetRotationSpeedZ2 = trampoline.write_call<5>(hook1.address() + 0x7A5 /* RELOCATION_OFFSET(0x5E4, 0x632) */, AIProcess_SetRotationSpeedZ2);
			_AIProcess_SetRotationSpeedZ3 = trampoline.write_branch<5>(hook2.address() + 0x44 /* RELOCATION_OFFSET(0x49, 0x49) */, AIProcess_SetRotationSpeedZ3);
		}

	private:
		static void AIProcess_SetRotationSpeedZ1(ConsoleRE::AIProcess* a_this, float a_rotationSpeed);
		static void AIProcess_SetRotationSpeedZ2(ConsoleRE::AIProcess* a_this, float a_rotationSpeed);
		static void AIProcess_SetRotationSpeedZ3(ConsoleRE::AIProcess* a_this, float a_rotationSpeed);
		static inline REL::Relocation<decltype(AIProcess_SetRotationSpeedZ1)> _AIProcess_SetRotationSpeedZ1;
		static inline REL::Relocation<decltype(AIProcess_SetRotationSpeedZ2)> _AIProcess_SetRotationSpeedZ2;
		static inline REL::Relocation<decltype(AIProcess_SetRotationSpeedZ3)> _AIProcess_SetRotationSpeedZ3;
	};

	class Actor_SetRotationHook
	{
	public:
		static void Hook()
		{
			auto& trampoline = API::GetTrampoline();
			REL::Relocation<uintptr_t> hook1(0x5BFEC0); // { RELOCATION_ID(32042, 32796) };  // 4EC300, 504B30  // synchronized anims
			REL::Relocation<uintptr_t> hook2(0x6DB4C0); // { RELOCATION_ID(36365, 37356) };  // 5D87F0, 5FD7E0
			REL::Relocation<uintptr_t> hook3(0x779960); // { RELOCATION_ID(38613, 39644) };  // 664C80, 68C160

			_Actor_SetRotationX  = trampoline.write_call<5>(hook1.address() + 0x5CE /* RELOCATION_OFFSET(0x4DC, 0x667)*/, Actor_SetRotationX);    // 4EC7DC
			_Actor_SetRotationZ1 = trampoline.write_call<5>(hook2.address() + 0xFBE /* RELOCATION_OFFSET(0x9C7, 0xA87)*/, Actor_SetRotationZ1);  // 5D91B7
			_Actor_SetRotationZ2 = trampoline.write_call<5>(hook3.address() + 0x288E /* RELOCATION_OFFSET(0x59A, 0x5C5)*/, Actor_SetRotationZ2);  // 66521A
		}

	private:
		static void Actor_SetRotationX(ConsoleRE::Actor* a_this, float a_angle);
		static void Actor_SetRotationZ1(ConsoleRE::Actor* a_this, float a_angle);
		static void Actor_SetRotationZ2(ConsoleRE::Actor* a_this, float a_angle);
		static inline REL::Relocation<decltype(Actor_SetRotationX)> _Actor_SetRotationX;
		static inline REL::Relocation<decltype(Actor_SetRotationZ1)> _Actor_SetRotationZ1;
		static inline REL::Relocation<decltype(Actor_SetRotationZ2)> _Actor_SetRotationZ2;
	};

	class EnemyHealthHook
	{
	public:
		static void Hook()
		{
			REL::Relocation<uintptr_t> EnemyHealthVtbl{ ConsoleRE::VTABLE_EnemyHealth[0] };
			_ProcessMessage = EnemyHealthVtbl.write_vfunc(0x3, ProcessMessage);
		}

	private:
		static bool ProcessMessage(uintptr_t a_enemyHealth, ConsoleRE::HUDData* a_hudData);
		static inline REL::Relocation<decltype(ProcessMessage)> _ProcessMessage;
	};

	class HeadtrackingHook
	{
	public:
		static void Hook()
		{
			auto& trampoline = API::GetTrampoline();
			REL::Relocation<uintptr_t> hook1(0x6F76B0); // { RELOCATION_ID(36984, 38009) };                                    // 60D300, 635270
			REL::Relocation<uintptr_t> hook2(0x70E600); // { RELOCATION_ID(36376, 37367) };                                    // 5D9BF0, 5FF320
			REL::Relocation<uintptr_t> hook3(0x715970); // { RELOCATION_ID(36220, 37200) };                                    // 5CFD60, 5F4320
			REL::Relocation<uintptr_t> hook4(0x6F9980); // { RELOCATION_ID(36540, 37541) };                                    // 5E8070, 60E050

			_SetHeadtrackTarget0 = trampoline.write_call<5>(hook1.address()  + 0x8A4 /* RELOCATION_OFFSET(0x592, 0x594) */, SetHeadtrackTarget0);    // 60D892
			_SetHeadtrackTarget4A = trampoline.write_call<5>(hook2.address() + 0xD9  /* RELOCATION_OFFSET(0xA3, 0xA3) */,   SetHeadtrackTarget4A);   // 5D9C93
			_SetHeadtrackTarget4B = trampoline.write_call<5>(hook3.address() + 0x650 /* RELOCATION_OFFSET(0x45C, 0x470) */, SetHeadtrackTarget4B);  // 5D01BC
			_SetHeadtrackTarget4C = trampoline.write_call<5>(hook3.address() + 0x778 /* RELOCATION_OFFSET(0x5BF, 0x5D3) */, SetHeadtrackTarget4C);  // 5D031F
			_SetHeadtrackTarget4D = trampoline.write_call<5>(hook4.address() + 0x166 /* RELOCATION_OFFSET(0x17E, 0x181) */, SetHeadtrackTarget4D);  // 5E81EE
		}

	private:
		static void SetHeadtrackTarget0(ConsoleRE::AIProcess* a_this, ConsoleRE::Actor* a_target);
		static void SetHeadtrackTarget4A(ConsoleRE::AIProcess* a_this, ConsoleRE::Actor* a_target);
		static void SetHeadtrackTarget4B(ConsoleRE::AIProcess* a_this, ConsoleRE::Actor* a_target);
		static void SetHeadtrackTarget4C(ConsoleRE::AIProcess* a_this, ConsoleRE::Actor* a_target);
		static void SetHeadtrackTarget4D(ConsoleRE::AIProcess* a_this, ConsoleRE::Actor* a_target);
		static inline REL::Relocation<decltype(SetHeadtrackTarget0)> _SetHeadtrackTarget0;
		static inline REL::Relocation<decltype(SetHeadtrackTarget4A)> _SetHeadtrackTarget4A;
		static inline REL::Relocation<decltype(SetHeadtrackTarget4B)> _SetHeadtrackTarget4B;
		static inline REL::Relocation<decltype(SetHeadtrackTarget4C)> _SetHeadtrackTarget4C;
		static inline REL::Relocation<decltype(SetHeadtrackTarget4D)> _SetHeadtrackTarget4D;
	};

	class NukeSetIsNPCHook
	{
	public:
		static void Hook()
		{
			auto& trampoline = API::GetTrampoline();

			REL::Relocation<uintptr_t> hook1(0xB6A980); // { RELOCATION_ID(55694, 56225) };  // 996FD0, 9BF1F0, bool papyrus wrapper
			REL::Relocation<uintptr_t> hook2(0xB6AA60); // { RELOCATION_ID(55695, 56226) };  // 997090, 9BF2B0, int papyrus wrapper
			// REL::Relocation<uintptr_t> hook{ RELOCATION_ID(32141, 32885) }; // 4F06E0, 5096B0

			_SetBool = trampoline.write_call<5>(hook1.address() + 0x58 /*0x4C*/, SetBool);
			_SetInt = trampoline.write_call<5>(hook2.address() + 0x58  /*0x4B*/, SetInt);
			// _SetBool = trampoline.write_call<5>(hook.address() + 0xE, SetBool);
		}

	private:
		static void SetBool(ConsoleRE::IAnimationGraphManagerHolder* a_this, ConsoleRE::BSFixedString* a_variableName, bool a_value);
		static void SetInt(ConsoleRE::IAnimationGraphManagerHolder* a_this, ConsoleRE::BSFixedString* a_variableName, int32_t a_value);

		static inline REL::Relocation<decltype(SetBool)> _SetBool;
		static inline REL::Relocation<decltype(SetInt)> _SetInt;
	};

	//
	class PlayerCameraHook // to fix Improved Camera breaking player pitch during target lock
	{
	public:
		static void Hook()
		{
			auto& trampoline = API::GetTrampoline();
			REL::Relocation<uintptr_t> hook1(0x9CBAD0); // { RELOCATION_ID(49852, 50784) };                             // 84AB90, 876700
			REL::Relocation<uintptr_t> hook2(0x9D13E0); // { RELOCATION_ID(49883, 50816) };                             // 84BCC0, 877970, EnterTweenMenuState

			_Update = trampoline.write_call<5>(hook1.address() + 0x373 /*RELOCATION_OFFSET(0x1A6, 0x1A6)*/, Update);
			_SetCameraState = trampoline.write_call<5>(hook2.address() + 0x99 /* RELOCATION_OFFSET(0x7C, 0x83) */, SetCameraState);
		}

	private:
		static void Update(ConsoleRE::TESCamera* a_this);
		static void SetCameraState(ConsoleRE::TESCamera* a_this, ConsoleRE::TESCameraState* a_newState);

		static inline REL::Relocation<decltype(Update)> _Update;
		static inline REL::Relocation<decltype(SetCameraState)> _SetCameraState;
	};

	class MainUpdateHook
	{
	public:
		static void Hook()
		{
			auto& trampoline = API::GetTrampoline();
			REL::Relocation<uintptr_t> hook(0x6A7640); // { RELOCATION_ID(35565, 36564) };  // 5B2FF0, 5D9F50, main update
			
			_Nullsub = trampoline.write_call<5>(hook.address() + 0x8F8 /*RELOCATION_OFFSET(0x748, 0xC26)*/, Nullsub);  // 5B3738, 5DAB76
		}

	private:
		static void Nullsub();
		static inline REL::Relocation<decltype(Nullsub)> _Nullsub;		
	};

	//
	class HorseAimHook
	{
	public:
		static void Hook()
		{
			auto& trampoline = API::GetTrampoline();
			REL::Relocation<uintptr_t> hook1(0x892210); // { RELOCATION_ID(42496, 43657) };  // 72FAC0, 75A890
			REL::Relocation<uintptr_t> hook2(0x9D2100); // { RELOCATION_ID(49960, 50896) };  // 84F490, 87B570
			REL::Relocation<uintptr_t> hook3(0x8B8CE0); // { RELOCATION_ID(43009, 44200) };  // 7516E0, 77EFD0, replace horse aim yaw

			_GetHorseCameraFreeRotationYaw = trampoline.write_call<5>(hook1.address() + 0x2A0 /* RELOCATION_OFFSET(0x17A, 0x1B5)*/, GetHorseCameraFreeRotationYaw);  // 72FC3A
			_Func = trampoline.write_call<5>(hook2.address() + 0x4C  /*RELOCATION_OFFSET(0x45, 0x45)*/, Func);  // 84F4D5
			_GetYaw = trampoline.write_call<5>(hook3.address() + 0x206/*RELOCATION_OFFSET(0x1C0, 0x1C0)*/, GetYaw);  // 7518A0
			// _GetMovementAgentPosition = trampoline.write_call<5>(hook1.address() + 0x22B, GetMovementAgentPosition); // 72FCEB - NPC Z offset after the location is set
		}


	private:
		static float* GetHorseCameraFreeRotationYaw(ConsoleRE::PlayerCamera* a_this);
		//static void GetMovementAgentPosition(ConsoleRE::Actor* a_this, ConsoleRE::NiPoint3& a_pos);
		static void Func(ConsoleRE::PlayerCamera* a_this);
		static float GetYaw(ConsoleRE::Actor* a_this);
		
		static inline REL::Relocation<decltype(GetHorseCameraFreeRotationYaw)> _GetHorseCameraFreeRotationYaw;
		//static inline REL::Relocation<decltype(GetMovementAgentPosition)> _GetMovementAgentPosition;
		static inline REL::Relocation<decltype(Func)> _Func;
		static inline REL::Relocation<decltype(GetYaw)> _GetYaw;
	};

	void Install();
}
