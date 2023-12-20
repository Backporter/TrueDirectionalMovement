#pragma once

#include "SmoothCamAPI.h"
#include "TrueHUDAPI.h"
#include "Widgets/TargetLockReticle.h"
#include <unordered_set>

namespace std
{
	template <>
	struct hash<ConsoleRE::ActorHandle>
	{
		uint32_t operator()(const ConsoleRE::ActorHandle& a_handle) const
		{
			uint32_t nativeHandle = const_cast<ConsoleRE::ActorHandle*>(&a_handle)->native_handle();  // ugh
			return nativeHandle;
		}
	};

	template <>
	struct hash<ConsoleRE::ObjectRefHandle>
	{
		uint32_t operator()(const ConsoleRE::ObjectRefHandle& a_handle) const
		{
			uint32_t nativeHandle = const_cast<ConsoleRE::ObjectRefHandle*>(&a_handle)->native_handle();
			return nativeHandle;
		}
	};
}

class DirectionalMovementHandler :
	public ConsoleRE::BSTEventSink<ConsoleRE::BSAnimationGraphEvent>
{
public:
	enum class AttackState : uint8_t
	{
		kNone = 0,
		kStart = 1,
		kMid = 2,
		kEnd = 3
	};

	using EventResult = ConsoleRE::BSEventNotifyControl;

	static DirectionalMovementHandler* GetSingleton();
	static void Register();

	// override BSTEventSink
	virtual EventResult ProcessEvent(const ConsoleRE::BSAnimationGraphEvent* a_event, ConsoleRE::BSTEventSource<ConsoleRE::BSAnimationGraphEvent>* a_eventSource) override;

	static void ResetControls();

	void Update();
	void UpdateDirectionalMovement();
	void UpdateFacingState();
	void UpdateFacingCrosshair();
	void UpdateDodgingState();
	void UpdateJumpingState();
	void UpdateSwimmingPitchOffset();
	void UpdateMountedArchery();
	void ProgressTimers();
	
	void UpdateProjectileTargetMap();

	void UpdateLeaning(ConsoleRE::Actor* a_actor, [[maybe_unused]] float a_deltaTime);

	void UpdateCameraAutoRotation();
	void ResetCameraRotationDelay();

	bool IsCrosshairVisible() const;
	void HideCrosshair();
	void ShowCrosshair();

	bool IsAiming() const;
	void SetIsAiming(bool a_bIsAiming);

	bool ProcessInput(ConsoleRE::NiPoint2& a_inputDirection, ConsoleRE::PlayerControlsData* a_playerControlsData);
	void SetDesiredAngleToTarget(ConsoleRE::PlayerCharacter* a_playerCharacter, ConsoleRE::ActorHandle a_target);
	void UpdateRotation(bool bForceInstant = false);
	void UpdateRotationLockedCam();
	void UpdateTweeningState();
	void UpdateAIProcessRotationSpeed(ConsoleRE::Actor* a_actor);
	void SetDesiredAIProcessRotationSpeed(float a_rotationSpeed);
	
	bool IFPV_IsFirstPerson() const;
	bool ImprovedCamera_IsFirstPerson() const;
	bool IsImprovedCameraInstalled() const;

	bool IsFreeCamera() const;
	bool Is360Movement() const;
	bool GetFreeCameraEnabled() const;
	bool HasMovementInput() const;	

	bool IsDodging() const;
	bool IsMagnetismActive() const;

	bool IsPlayerAnimationDriven() const;
	bool IsTDMRotationLocked() const;

	AttackState GetAttackState() const;
	void SetAttackState(AttackState a_state);

	bool IsCameraResetting() const;
	void ResetCamera();

	void ResetDesiredAngle();

	float GetYawDelta() const;
	void ResetYawDelta();

	ConsoleRE::NiPoint2 GetActualInputDirection() const;

	enum class Direction
	{
		kInvalid = 0,
		kLeft = 1 << 0,
		kRight = 1 << 1,
		kForward = 1 << 2,
		kBack = 1 << 3,
		kUp = 1 << 4,
		kDown = 1 << 5
	};

	enum class TargetLockSelectionMode : uint32_t
	{
		kClosest = 0,
		kCenter = 1,
		kCombined = 2
	};

	SKSE::stl::enumeration<Direction, uint8_t> _pressedDirections;

	bool ToggleTargetLock(bool bEnable, bool bPressedManually = false);
	ConsoleRE::ActorHandle GetTarget() const;
	ConsoleRE::NiPointer<ConsoleRE::NiAVObject> GetTargetPoint() const;
	ConsoleRE::NiPoint3 GetTargetPosition() const;
	void ClearTargets();

	void OverrideControllerBufferDepth(bool a_override);

	float GetTargetLockDistanceRaceSizeMultiplier(ConsoleRE::TESRace* a_race) const;
	bool CheckCurrentTarget(ConsoleRE::ActorHandle a_target, bool bInstantLOS = false);
	void UpdateTargetLock();

	bool IsActorValidTarget(ConsoleRE::ActorPtr a_actor, bool a_bCheckDistance = false) const;

	ConsoleRE::ActorHandle FindTarget(TargetLockSelectionMode a_mode, bool a_bSkipCurrent = true);
	void SwitchTarget(Direction a_direction);
	bool SwitchTargetPoint(Direction a_direction);
	ConsoleRE::ActorHandle SwitchScreenTarget(Direction a_direction);
	std::vector<ConsoleRE::NiPointer<ConsoleRE::NiAVObject>> GetTargetPoints(ConsoleRE::ActorHandle a_actorHandle) const;
	ConsoleRE::NiPointer<ConsoleRE::NiAVObject> GetBestTargetPoint(ConsoleRE::ActorHandle a_actorHandle) const;

	bool SetDesiredAngleToMagnetismTarget();
	
	float GetCurrentSwimmingPitchOffset() const;
	void SetDesiredSwimmingPitchOffset(float a_value);

	void SetTarget(ConsoleRE::ActorHandle a_target);
	void SetSoftTarget(ConsoleRE::ActorHandle a_softTarget);
	void SetTargetPoint(ConsoleRE::NiPointer<ConsoleRE::NiAVObject> a_targetPoint);

	ConsoleRE::NiAVObject* GetProjectileTargetPoint(ConsoleRE::ObjectRefHandle a_projectileHandle) const;
	void AddProjectileTarget(ConsoleRE::ObjectRefHandle a_projectileHandle, ConsoleRE::NiPointer<ConsoleRE::NiAVObject> a_targetPoint);
	void RemoveProjectileTarget(ConsoleRE::ObjectRefHandle a_projectileHandle);

	void AddTargetLockReticle(ConsoleRE::ActorHandle a_target, ConsoleRE::NiPointer<ConsoleRE::NiAVObject> a_targetPoint);
	void ReticleRemoved();
	void RemoveTargetLockReticle();
	
	void SetHeadtrackTarget(ConsoleRE::HighProcessData::HEAD_TRACK_TYPE a_headtrackType, ConsoleRE::TESObjectREFR* a_target);

	void UpdateCameraHeadtracking();

	void SetPreviousHorseAimAngle(float a_angle);
	void SetCurrentHorseAimAngle(float a_angle);
	bool GetCurrentlyMountedAiming() const;
	void SetCurrentlyMountedAiming(bool a_aiming);
	void UpdateHorseAimDirection();
	void SetNewHorseAimDirection(float a_angle);
	float GetCurrentHorseAimAngle() const;

	void SetLastInputDirection(ConsoleRE::NiPoint2& a_inputDirection);
	bool CheckInputDot(float a_dot) const;
	bool DetectInputAnalogStickBounce() const;

	void SetCameraStateBeforeTween(ConsoleRE::CameraStates::CameraState a_cameraState);

	ConsoleRE::NiPoint3 GetCameraRotation();

	void LookAtTarget(ConsoleRE::ActorHandle a_target);

	bool ShouldFaceTarget() const;
	bool ShouldFaceCrosshair() const;

	bool HasTargetLocked() const;

	float GetDialogueHeadtrackTimer() const;
	void RefreshDialogueHeadtrackTimer();
	float GetCameraHeadtrackTimer() const;
	void RefreshCameraHeadtrackTimer();

	void Initialize();
	void OnPreLoadGame();

	void OnSettingsUpdated();	

	void InitCameraModsCompatibility();

	static bool IsBehaviorPatchInstalled(ConsoleRE::TESObjectREFR* a_ref);
	static bool IsMountedArcheryPatchInstalled(ConsoleRE::TESObjectREFR* a_ref);

	bool GetPlayerIsNPC() const;
	void SetPlayerIsNPC(bool a_enable);

	void UpdatePlayerPitch();	

	static void RegisterSmoothCamCallback();
	static void RequestAPIs();

	static inline bool bRegisteredSmoothCamCallback = false;
	static inline SmoothCamAPI::IVSmoothCam3* g_SmoothCam = nullptr;
	static inline TRUEHUD_API::IVTrueHUD3* g_trueHUD = nullptr;
	std::atomic_bool _bReticleRemoved{ false };

	bool GetForceDisableDirectionalMovement() const;
	bool GetForceDisableHeadtracking() const;
	bool GetYawControl() const;
	void SetForceDisableDirectionalMovement(bool a_disable);
	void SetForceDisableHeadtracking(bool a_disable);
	void SetYawControl(bool a_enable, float a_yawRotationSpeedMultiplier = 0);
	void SetPlayerYaw(float a_yaw);

	void PapyrusDisableDirectionalMovement(const char* a_modName, bool a_bDisable);
	void PapyrusDisableHeadtracking(const char* a_modName, bool a_bDisable);

	bool IsACCInstalled() const { return _bACCInstalled; }
	bool IsICInstalled() const { return _bICInstalled; }

private:
	using Lock = std::recursive_mutex;
	using Locker = std::lock_guard<Lock>;

	DirectionalMovementHandler();
	DirectionalMovementHandler(const DirectionalMovementHandler&) = delete;
	DirectionalMovementHandler(DirectionalMovementHandler&&) = delete;
	~DirectionalMovementHandler() = default;

	DirectionalMovementHandler& operator=(const DirectionalMovementHandler&) = delete;
	DirectionalMovementHandler& operator=(DirectionalMovementHandler&&) = delete;

	mutable Lock _lock;

	float _defaultControllerBufferDepth = -1.f;
	float _defaultAcrobatics = -1.f;
	
	bool _bMagnetismActive = false;
	bool _bCurrentlyTurningToCrosshair = false;

	float _desiredAngle = -1.f;

	ConsoleRE::NiPoint2 _actualInputDirection;

	bool _bDirectionalMovement = false;
	bool _bShouldFaceCrosshair = false;
	bool _bShouldFaceTarget = false;

	bool _bUpdatePlayerPitch = false;
	float _desiredPlayerPitch;

	bool _bResetCamera = false;
	float _desiredCameraAngleX;
	float _desiredCameraAngleY;		

	bool _bIsTweening = false;
	float _yawDelta = 0.f;
	
	float _desiredAIProcessRotationSpeed = 0.f;
	Direction _lastTargetSwitchDirection = Direction::kInvalid;

	float _previousHorseAimAngle = 0.f;
	float _horseAimAngle = 0.f;
	Direction _currentHorseAimDirection = Direction::kForward;
	bool _currentlyMountedAiming = false;

	ConsoleRE::CameraStates::CameraState _cameraStateBeforeTween;

	// for analog bounce fix
	static constexpr float _analogBounceDotThreshold = 0.25f;
	static constexpr size_t _inputBufferSize = 5;
	std::deque<ConsoleRE::NiPoint2> _lastInputs;

	float _lastTargetSwitchTimer = 0.f;
	float _lastLOSTimer = 0.f;
	float _dialogueHeadtrackTimer = 0.f;
	float _faceCrosshairTimer = 0.f;
	float _aimingTimer = 0.f;
	float _cameraHeadtrackTimer = 0.f;
	float _cameraRotationDelayTimer = 0.f;
	float _tutorialHintTimer = 0.f;

	bool _bCrosshairIsHidden = false;
	bool _bIsAiming = false;

	float _desiredSwimmingPitchOffset = 0.f;
	float _currentSwimmingPitchOffset = 0.f;

	float _currentAutoCameraRotationSpeed = 0.f;
	
	static constexpr float _lostSightAllowedDuration = 2.f;
	static constexpr float _meleeMagnetismRange = 250.f;
	static constexpr float _faceCrosshairDuration = 0.4f;
	static constexpr float _aimingDuration = 0.1f;
	static constexpr float _targetLockDistanceHysteresis = 1.05f;
	static constexpr float _hintDuration = 5.f;

	bool _playerIsNPC = false;

	bool _bHasMovementInput = false;
	bool _bIsDodging = false;
	bool _bJustDodged = false;
	AttackState _attackState;

	bool _bForceDisableDirectionalMovement = false;
	std::unordered_set<std::string> _papyrusDisableDirectionalMovement{};
    bool _bForceDisableHeadtracking = false;
	std::unordered_set<std::string> _papyrusDisableHeadtracking{};
	bool _bYawControlledByPlugin = false;
	float _controlledYawRotationSpeedMultiplier = 0;

	ConsoleRE::ActorHandle _target;
	ConsoleRE::ActorHandle _softTarget;
	ConsoleRE::ObjectRefHandle _dialogueSpeaker;
	ConsoleRE::NiPointer<ConsoleRE::NiAVObject> _currentTargetPoint;
	
	std::unordered_map<ConsoleRE::ObjectRefHandle, ConsoleRE::NiPointer<ConsoleRE::NiAVObject>> _projectileTargets;

	// Compatibility
	ConsoleRE::TESGlobal* _IFPV_IsFirstPerson = nullptr;
	bool* _ImprovedCamera_IsFirstPerson = nullptr;
	bool _bACCInstalled = false;
	bool _bICInstalled = false;
	bool _bControlsTrueHUDTarget = false;

	bool _mountedArcheryRequestedSmoothCamCrosshair = false;
	bool _targetLockRequestedSmoothCamCrosshair = false;

	std::weak_ptr<Scaleform::TargetLockReticle> _targetLockReticle;
};
