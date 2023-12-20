#include "Hooks.h"
#include "Settings.h"
#include "DirectionalMovementHandler.h"
#include "Offsets.h"
#include "Utils.h"
#include "RayCast.h"

namespace Hooks
{
	struct SaveCamera
	{
		enum class RotationType : uint8_t
		{
			kNone,
			kFirstPerson,
			kThirdPerson,
			kHorse
		};
		
		RotationType rotationType = RotationType::kNone;
		ConsoleRE::NiPoint2 rotation { 0.f, 0.f };
		bool bPosOffsetSaved = false;
		ConsoleRE::NiPoint3 posOffset{ 0.f, 0.f, 0.f };
		bool bZoomSaved = false;
		float zoom = 0.f;
		float pitchZoomOffset = 0.f;

		void SaveYaw(float a_x, RotationType a_rotationType)
		{
			rotation.x = a_x;
			rotationType = a_rotationType;
		}

		void SavePitch(float a_y, RotationType a_rotationType)
		{
			rotation.y = a_y;
			rotationType = a_rotationType;
		}

		void SaveRotation(ConsoleRE::NiPoint2 a_xy, RotationType a_rotationType)
		{
			rotation = a_xy;
			rotationType = a_rotationType;
		}

		void SavePosOffset(ConsoleRE::NiPoint3& a_offset)
		{
			posOffset = a_offset;
			bPosOffsetSaved = true;
		}

		float ConsumeYaw()
		{
			rotationType = RotationType::kNone;
			return rotation.x;
		}

		float ConsumePitch()
		{
			rotationType = RotationType::kNone;
			return rotation.y;
		}

		ConsoleRE::NiPoint2& ConsumeRotation()
		{
			rotationType = RotationType::kNone;
			return rotation;
		}
		
		ConsoleRE::NiPoint3 ConsumePosOffset()
		{
			bPosOffsetSaved = false;
			return posOffset;
		}

		void SaveZoom(float a_zoomOffset, float a_pitchZoomOffset)
		{
			zoom = a_zoomOffset;
			pitchZoomOffset = a_pitchZoomOffset;
			bZoomSaved = true;
		}

		void ConsumeZoom(float& a_outZoomOffset, float& a_outPitchZoomOffset)
		{
			bZoomSaved = false;
			a_outZoomOffset = zoom;
			a_outPitchZoomOffset = pitchZoomOffset;
		}

	} 
	
	static savedCamera;

	void Install()
	{
		xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "Hooking...");

		MovementHook::Hook();
		GamepadHook::Hook();
		LookHook::Hook();
		TogglePOVHook::Hook();
		FirstPersonStateHook::Hook();
		ThirdPersonStateHook::Hook();
		HorseCameraStateHook::Hook();
		TweenMenuCameraStateHook::Hook();
		VATSCameraStateHook::Hook();
		PlayerCameraTransitionStateHook::Hook();
		MovementHandlerAgentPlayerControlsHook::Hook();
		ProjectileHook::Hook();
		CharacterHook::Hook();
		PlayerCharacterHook::Hook();
		PlayerControlsHook::Hook();
		AIProcess_SetRotationSpeedZHook::Hook();
		Actor_SetRotationHook::Hook();
		EnemyHealthHook::Hook();
		HeadtrackingHook::Hook();
		NukeSetIsNPCHook::Hook();
		PlayerCameraHook::Hook();
		MainUpdateHook::Hook();
		HorseAimHook::Hook();

		xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "...success");
	}

	void MovementHook::ProcessThumbstick(ConsoleRE::MovementHandler* a_this, ConsoleRE::ThumbstickEvent* a_event, ConsoleRE::PlayerControlsData* a_data)
	{
		// save the original values
        ConsoleRE::NiPoint2 savedMoveInput = a_data->moveInputVec;

		// call original function so other plugins can hook this vfunc properly
        _ProcessThumbstick(a_this, a_event, a_data);

		// save new values
		ConsoleRE::NiPoint2 newMoveInput = a_data->moveInputVec;

		// restore original values before we do our logic
		a_data->moveInputVec = savedMoveInput;
		
		bool bHandled = false;
		auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();
		auto playerCharacter = ConsoleRE::PlayerCharacter::GetSingleton();
		if (a_event && directionalMovementHandler->IsFreeCamera() && a_event->IsLeft() && playerCharacter && !playerCharacter->IsOnMount())
		{
			ConsoleRE::NiPoint2 inputDirection(a_event->xValue, a_event->yValue);
			bHandled = directionalMovementHandler->ProcessInput(inputDirection, a_data);
		}

		if (!bHandled)
		{
			// if our logic didn't handle the input, return to the values that were set by running the original vfunc
            a_data->moveInputVec = newMoveInput;
			
			if (Settings::bThumbstickBounceFix) 
			{
				directionalMovementHandler->SetLastInputDirection(a_data->moveInputVec);
			}
		}
	}

	void MovementHook::ProcessButton(ConsoleRE::MovementHandler* a_this, ConsoleRE::ButtonEvent* a_event, ConsoleRE::PlayerControlsData* a_data)
	{
        // save the original values
        ConsoleRE::NiPoint2 savedMoveInput = a_data->moveInputVec;
        bool savedAutoMove = a_data->autoMove;
		
		// call original function so other plugins can hook this vfunc properly
        _ProcessButton(a_this, a_event, a_data);

		// save new values
		ConsoleRE::NiPoint2 newMoveInput = a_data->moveInputVec;
        bool newAutoMove = a_data->autoMove;

		// restore original values before we do our logic
		a_data->moveInputVec = savedMoveInput;
        a_data->autoMove = savedAutoMove;
		
		bool bHandled = false;
		DirectionalMovementHandler* directionalMovementHandler = DirectionalMovementHandler::GetSingleton();
		auto pressedDirections = &directionalMovementHandler->_pressedDirections;
		auto playerCharacter = ConsoleRE::PlayerCharacter::GetSingleton();
		if (a_event && directionalMovementHandler->IsFreeCamera() && playerCharacter && !playerCharacter->IsOnMount())
		{
			auto& userEvent = a_event->QUserEvent();
			auto userEvents = ConsoleRE::UserEvents::GetSingleton();

			bool bRelevant = false;

			ConsoleRE::NiPoint2 inputDirection(0.f, 0.f);

			if (userEvent == userEvents->forward) {
				a_event->IsPressed() ? pressedDirections->set(DirectionalMovementHandler::Direction::kForward) : pressedDirections->reset(DirectionalMovementHandler::Direction::kForward);
				bRelevant = true;
			} else if (userEvent == userEvents->back) {
				a_event->IsPressed() ? pressedDirections->set(DirectionalMovementHandler::Direction::kBack) : pressedDirections->reset(DirectionalMovementHandler::Direction::kBack);
				bRelevant = true;
			} else if (userEvent == userEvents->strafeLeft) {
				a_event->IsPressed() ? pressedDirections->set(DirectionalMovementHandler::Direction::kLeft) : pressedDirections->reset(DirectionalMovementHandler::Direction::kLeft);
				bRelevant = true;
			} else if (userEvent == userEvents->strafeRight) {
				a_event->IsPressed() ? pressedDirections->set(DirectionalMovementHandler::Direction::kRight) : pressedDirections->reset(DirectionalMovementHandler::Direction::kRight);
				bRelevant = true;
			}

			if (bRelevant)
			{	
				if (pressedDirections->any(DirectionalMovementHandler::Direction::kForward)) {
					inputDirection.y += 1.f;
				}
				if (pressedDirections->any(DirectionalMovementHandler::Direction::kBack)) {
					inputDirection.y -= 1.f;
				}
				if (pressedDirections->any(DirectionalMovementHandler::Direction::kRight)) {
					inputDirection.x += 1.f;
				} 
				if (pressedDirections->any(DirectionalMovementHandler::Direction::kLeft)) {
					inputDirection.x -= 1.f;
				}

				bHandled = directionalMovementHandler->ProcessInput(inputDirection, a_data);
			}
		}

		if (!bHandled)
		{
            // if our logic didn't handle the input, return to the values that were set by running the original vfunc
            a_data->moveInputVec = newMoveInput;
            a_data->autoMove = newAutoMove;
			
			*pressedDirections = DirectionalMovementHandler::Direction::kInvalid;
			if (Settings::bThumbstickBounceFix) {
				directionalMovementHandler->SetLastInputDirection(a_data->moveInputVec);
			}
		}
	}

	void GamepadHook::ProcessInput(ConsoleRE::BSOrbisGamepadDevice* a_this, int32_t a_rawX, int32_t a_rawY, float a_deadzoneMin, float a_deadzoneMax, float& a_outX, float& a_outY)
	{
		_ProcessInput(a_this, a_rawX, a_rawY, a_deadzoneMin, a_deadzoneMax, a_outX, a_outY);

		if (!Settings::bOverrideControllerDeadzone) {

			return;
		}

		a_this->ProcessRawInput(a_rawX, a_rawY, a_outX, a_outY);

		ConsoleRE::NiPoint2 normalizedInputDirection{ a_outX, a_outY };
		float inputLength = normalizedInputDirection.Unitize();

		// deadzone
		if (inputLength < Settings::fControllerRadialDeadzone) 
		{
			a_outX = 0.f;
			a_outY = 0.f;
			return;
		}

		// radial deadzone
		a_outX = normalizedInputDirection.x * Remap(inputLength, Settings::fControllerRadialDeadzone, 1.f, 0.f, 1.f);
		a_outY = normalizedInputDirection.y * Remap(inputLength, Settings::fControllerRadialDeadzone, 1.f, 0.f, 1.f);

		// axial deadzone
		float absX = fabs(a_outX);
		float absY = fabs(a_outY);

		ConsoleRE::NiPoint2 deadzone;
		deadzone.x = Settings::fControllerAxialDeadzone * absY;
		deadzone.y = Settings::fControllerAxialDeadzone * absX;
		ConsoleRE::NiPoint2 sign;
		sign.x = a_outX < 0.f ? -1.f : 1.f;
		sign.y = a_outY < 0.f ? -1.f : 1.f;
		if (absX > deadzone.x) {
			a_outX = sign.x * Remap(absX, deadzone.x, 1.f, 0.f, 1.f);
		} else 
		{
			a_outX = 0.f;
		}
		if (absY > deadzone.y) {
			a_outY = sign.y * Remap(absY, deadzone.y, 1.f, 0.f, 1.f);
		} else {
			a_outY = 0.f;
		}
	}

	static bool bTargetRecentlySwitched;

	void LookHook::ProcessThumbstick(ConsoleRE::LookHandler* a_this, ConsoleRE::ThumbstickEvent* a_event, ConsoleRE::PlayerControlsData* a_data)
	{
		auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();
		if (a_event && a_event->IsRight() && directionalMovementHandler->HasTargetLocked() && !directionalMovementHandler->ShouldFaceCrosshair()) 
		{
			if (!Settings::bTargetLockUseRightThumbstick)
			{
				return;  // ensure lock camera movement during lockon
			}

			float absX = fabs(a_event->xValue);
			float absY = fabs(a_event->yValue);

			if (absX + absY > 0.1f && !bTargetRecentlySwitched) {
				if (absX > absY) {
					if (a_event->xValue > 0) {
						directionalMovementHandler->SwitchTarget(DirectionalMovementHandler::Direction::kRight);
					} else {
						directionalMovementHandler->SwitchTarget(DirectionalMovementHandler::Direction::kLeft);
					}
				} else {
					if (a_event->yValue > 0) {
						directionalMovementHandler->SwitchTarget(DirectionalMovementHandler::Direction::kUp);
					} else {
						directionalMovementHandler->SwitchTarget(DirectionalMovementHandler::Direction::kDown);
					}
				}

				bTargetRecentlySwitched = true;
			} 
			else if (absX + absY <= 0.1f)
			{
				bTargetRecentlySwitched = false;
			}
		}
		else
		{
			bTargetRecentlySwitched = false;
			if (Settings::bCameraHeadtracking && Settings::fCameraHeadtrackingDuration > 0.f) {
				directionalMovementHandler->RefreshCameraHeadtrackTimer();
			}

			if (directionalMovementHandler->IsCameraResetting()) {
				return; // ensure lock camera movement during camera reset
			}

			if (Settings::uAdjustCameraYawDuringMovement > CameraAdjustMode::kDisable && Settings::fCameraAutoAdjustDelay > 0.f) {
				directionalMovementHandler->ResetCameraRotationDelay();
			}

			_ProcessThumbstick(a_this, a_event, a_data);
		}
	}

	void LookHook::ProcessMouseMove(ConsoleRE::LookHandler* a_this, ConsoleRE::MouseMoveEvent* a_event, ConsoleRE::PlayerControlsData* a_data)
	{
		auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();
		if (a_event && directionalMovementHandler->HasTargetLocked() && !directionalMovementHandler->ShouldFaceCrosshair())
		{
			if (!Settings::bTargetLockUseMouse)
			{
				return; // ensure lock camera movement during lockon
			}

			int32_t absX = abs(a_event->mouseInputX);
			int32_t absY = abs(a_event->mouseInputY);

			if (absX + absY > static_cast<int32_t>(Settings::uTargetLockMouseSensitivity))
			{
				if (absX > absY)
				{
					if (a_event->mouseInputX > 0) {
						directionalMovementHandler->SwitchTarget(DirectionalMovementHandler::Direction::kRight);
					} else {
						directionalMovementHandler->SwitchTarget(DirectionalMovementHandler::Direction::kLeft);
					}
				}
				else 
				{
					if (a_event->mouseInputY > 0) {
						directionalMovementHandler->SwitchTarget(DirectionalMovementHandler::Direction::kDown);
					} else {
						directionalMovementHandler->SwitchTarget(DirectionalMovementHandler::Direction::kUp);
					}
				}

				bTargetRecentlySwitched = true;
			} 
			else if (absX + absY <= static_cast<int32_t>(Settings::uTargetLockMouseSensitivity))
			{
				bTargetRecentlySwitched = false;
			}
		}
		else
		{
			bTargetRecentlySwitched = false;
			if (Settings::bCameraHeadtracking && Settings::fCameraHeadtrackingDuration > 0.f) {
				directionalMovementHandler->RefreshCameraHeadtrackTimer();
			}

			if (directionalMovementHandler->IsCameraResetting()) {
				return;  // ensure lock camera movement during camera reset
			}

			if (Settings::uAdjustCameraYawDuringMovement > CameraAdjustMode::kDisable && Settings::fCameraAutoAdjustDelay > 0.f) {
				directionalMovementHandler->ResetCameraRotationDelay();
			}

			_ProcessMouseMove(a_this, a_event, a_data);
		}
	}

	static bool bPressedPOVToUnlock = false;
	static bool bInTargetLockWindow = false;

	void TogglePOVHook::ProcessButton(ConsoleRE::TogglePOVHandler* a_this, ConsoleRE::ButtonEvent* a_event, ConsoleRE::PlayerControlsData* a_data)
	{
		if (a_event && BSInputDeviceManager_IsUsingGamepad(ConsoleRE::BSInputDeviceManager::GetSingleton()) ? Settings::bTargetLockUsePOVSwitchGamepad : Settings::bTargetLockUsePOVSwitchKeyboard) {
			auto& userEvent = a_event->QUserEvent();
			auto userEvents = ConsoleRE::UserEvents::GetSingleton();

			if (userEvent == userEvents->togglePOV)
			{
				auto ui = ConsoleRE::UI::GetSingleton();
				auto controlMap = ConsoleRE::ControlMap::GetSingleton();
				if (!ui->GameIsPaused() && controlMap->IsMovementControlsEnabled()) {
					auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();
					if (directionalMovementHandler->HasTargetLocked()) {
						if (a_event->IsDown()) {
							directionalMovementHandler->ToggleTargetLock(false, true);
							bPressedPOVToUnlock = true;
						}
						return;
					} else {
						if (a_event->HeldDuration() < Settings::fTargetLockPOVHoldDuration) {
							if (a_event->IsDown()) {
								bInTargetLockWindow = true;
							}

							if (a_event->IsUp()) {
								if (!bPressedPOVToUnlock) {
									directionalMovementHandler->ToggleTargetLock(true, true);
								}
								bPressedPOVToUnlock = false;
								bInTargetLockWindow = false;
							}

							return;
						} else { // held duration longer than the threshold from settings
							if (bInTargetLockWindow) {
								bInTargetLockWindow = false;
								a_event->heldDownSecs = 0.f;

								auto playerCamera = ConsoleRE::PlayerCamera::GetSingleton();
								if (playerCamera->currentState && playerCamera->currentState->id == ConsoleRE::CameraStates::kFirstPerson) {
									a_event->value = 0.f;
								}
							}
							bPressedPOVToUnlock = false;
						}
					}
				}
			}
		}

		_ProcessButton(a_this, a_event, a_data);
	}

	void FirstPersonStateHook::OnEnterState(ConsoleRE::FirstPersonState* a_this)
	{
		auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();
		if (directionalMovementHandler->GetFreeCameraEnabled()) {
			auto playerCharacter = ConsoleRE::PlayerCharacter::GetSingleton();
			if (playerCharacter) {
				// turn character towards where the camera was looking in third person state before entering first person state
				if (savedCamera.rotationType == SaveCamera::RotationType::kThirdPerson) {
					auto x = savedCamera.ConsumeYaw();
					if (playerCharacter->AsActorState()->actorState1.sitSleepState == ConsoleRE::SIT_SLEEP_STATE::kNormal) {  // don't do this while sitting, sleeping etc.
						playerCharacter->SetRotationZ(x);
					}
				}
				savedCamera.bZoomSaved = false;

				directionalMovementHandler->ResetDesiredAngle();
			}
		}

		_OnEnterState(a_this);
	}

	void FirstPersonStateHook::OnExitState(ConsoleRE::FirstPersonState* a_this)
	{
		auto playerCharacter = ConsoleRE::PlayerCharacter::GetSingleton();
		
		if (playerCharacter)
		{
			savedCamera.SaveYaw(playerCharacter->data.angle.z, SaveCamera::RotationType::kFirstPerson);
		}
		
		_OnExitState(a_this);
	}

	void FirstPersonStateHook::ProcessButton(ConsoleRE::FirstPersonState* a_this, ConsoleRE::ButtonEvent* a_event, ConsoleRE::PlayerControlsData* a_data)
	{
		auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();
		if (a_event && directionalMovementHandler->HasTargetLocked() && Settings::bTargetLockUseScrollWheel)
		{
			auto& userEvent = a_event->QUserEvent();
			auto userEvents = ConsoleRE::UserEvents::GetSingleton();

			if (userEvent == userEvents->zoomIn) {
				directionalMovementHandler->SwitchTarget(DirectionalMovementHandler::Direction::kLeft);
				return;
			} else if (userEvent == userEvents->zoomOut) {
				directionalMovementHandler->SwitchTarget(DirectionalMovementHandler::Direction::kRight);
				return;
			}
		}

		if (a_event && BSInputDeviceManager_IsUsingGamepad(ConsoleRE::BSInputDeviceManager::GetSingleton()) ? Settings::bTargetLockUsePOVSwitchGamepad : Settings::bTargetLockUsePOVSwitchKeyboard) {
			auto& userEvent = a_event->QUserEvent();
			auto userEvents = ConsoleRE::UserEvents::GetSingleton();

			if (userEvent == userEvents->togglePOV && a_event->IsUp() && a_event->HeldDuration() < Settings::fTargetLockPOVHoldDuration) {
				return;
			}
		}

		_ProcessButton(a_this, a_event, a_data);
	}

	void ThirdPersonStateHook::OnEnterState(ConsoleRE::ThirdPersonState* a_this)
	{
		_OnEnterState(a_this);

		if (DirectionalMovementHandler::GetSingleton()->GetFreeCameraEnabled()) {
			if (savedCamera.rotationType == SaveCamera::RotationType::kHorse) {
				a_this->freeRotation.x = savedCamera.ConsumeYaw();
			}

			DirectionalMovementHandler::GetSingleton()->ResetDesiredAngle();
		}
	}

	void ThirdPersonStateHook::OnExitState(ConsoleRE::ThirdPersonState* a_this)
	{
		if (DirectionalMovementHandler::GetSingleton()->GetFreeCameraEnabled()) {
			ConsoleRE::Actor* cameraTarget = nullptr;
			cameraTarget = static_cast<ConsoleRE::PlayerCamera*>(a_this->camera)->cameraTarget.get().get();
			auto playerCharacter = ConsoleRE::PlayerCharacter::GetSingleton();
			
			ConsoleRE::NiPoint2 rot;
			rot.x = playerCharacter->data.angle.z + a_this->freeRotation.x;
			rot.y = playerCharacter->data.angle.x + a_this->freeRotation.y;
			savedCamera.SaveRotation(rot, SaveCamera::RotationType::kThirdPerson);
			savedCamera.SaveZoom(a_this->currentZoomOffset, a_this->pitchZoomOffset);
			savedCamera.SavePosOffset(a_this->posOffsetActual);
		}

		_OnExitState(a_this);
	}

	void ThirdPersonStateHook::SetFreeRotationMode(ConsoleRE::ThirdPersonState* a_this, bool a_weaponSheathed)
	{
		auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();

		//directionalMovementHandler->Update();

		if (directionalMovementHandler->IsFreeCamera())
		{
			ConsoleRE::Actor* cameraTarget = nullptr;
			cameraTarget = static_cast<ConsoleRE::PlayerCamera*>(a_this->camera)->cameraTarget.get().get();

			bool bHasTargetLocked = directionalMovementHandler->HasTargetLocked();

			if (cameraTarget) {
				a_this->freeRotationEnabled = true;
				directionalMovementHandler->UpdateAIProcessRotationSpeed(cameraTarget);  // because the game is skipping the original call while in freecam

				if (!bHasTargetLocked) {
					auto actorState = cameraTarget->AsActorState();

					float pitchDelta = -a_this->freeRotation.y;

					bool bIsSwimming = actorState->IsSwimming();
					// swimming pitch fix and swim up/down buttons handling
					if (bIsSwimming) {
						float currentPitch = cameraTarget->data.angle.x;

						if (directionalMovementHandler->_pressedDirections.any(DirectionalMovementHandler::Direction::kUp) || directionalMovementHandler->_pressedDirections.any(DirectionalMovementHandler::Direction::kDown)) {
							if (directionalMovementHandler->_pressedDirections.any(DirectionalMovementHandler::Direction::kUp)) {
								directionalMovementHandler->SetDesiredSwimmingPitchOffset(directionalMovementHandler->HasMovementInput() ? -PI / 4 : -PI / 2);
							}
							if (directionalMovementHandler->_pressedDirections.any(DirectionalMovementHandler::Direction::kDown)) {
								directionalMovementHandler->SetDesiredSwimmingPitchOffset(directionalMovementHandler->HasMovementInput() ? PI / 4 : PI / 2);
							}
							auto playerControls = ConsoleRE::PlayerControls::GetSingleton();
							playerControls->data.moveInputVec = ConsoleRE::NiPoint2(0.f, 1.f);
						} else if (directionalMovementHandler->_pressedDirections.underlying()) {
							directionalMovementHandler->SetDesiredSwimmingPitchOffset((currentPitch - a_this->freeRotation.y) * cos(fabs(a_this->freeRotation.x)));
						} else {
							directionalMovementHandler->SetDesiredSwimmingPitchOffset(0.f);
						}

						float desiredPitch = directionalMovementHandler->GetCurrentSwimmingPitchOffset();
						pitchDelta = desiredPitch - currentPitch;
						auto characterController = cameraTarget->GetCharController();
						if (characterController) {
							characterController->pitchAngle = desiredPitch;
						}
					}

					bool bMoving = actorState->actorState1.movingBack ||
					               actorState->actorState1.movingForward ||
					               actorState->actorState1.movingRight ||
					               actorState->actorState1.movingLeft;

					if (bMoving || !a_weaponSheathed) {
						//cameraTarget->SetRotationX(cameraTarget->data.angle.x + pitchDelta);
						cameraTarget->data.angle.x += pitchDelta;
						a_this->freeRotation.y += pitchDelta;
					}
				}
			}
		} else {
			_SetFreeRotationMode(a_this, a_weaponSheathed);
		}
	}

	void ThirdPersonStateHook::ProcessButton(ConsoleRE::ThirdPersonState* a_this, ConsoleRE::ButtonEvent* a_event, ConsoleRE::PlayerControlsData* a_data)
	{
		auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();
		if (a_event && directionalMovementHandler->HasTargetLocked() && Settings::bTargetLockUseScrollWheel) {
			auto& userEvent = a_event->QUserEvent();
			auto userEvents = ConsoleRE::UserEvents::GetSingleton();

			if (userEvent == userEvents->zoomIn) {
				directionalMovementHandler->SwitchTarget(DirectionalMovementHandler::Direction::kLeft);
				return;
			} else if (userEvent == userEvents->zoomOut) {
				directionalMovementHandler->SwitchTarget(DirectionalMovementHandler::Direction::kRight);
				return;
			}
		}

		if (a_event && BSInputDeviceManager_IsUsingGamepad(ConsoleRE::BSInputDeviceManager::GetSingleton()) ? Settings::bTargetLockUsePOVSwitchGamepad : Settings::bTargetLockUsePOVSwitchKeyboard)
		{
			auto& userEvent = a_event->QUserEvent();
			auto userEvents = ConsoleRE::UserEvents::GetSingleton();

			if (userEvent == userEvents->togglePOV && a_event->IsUp() && a_event->HeldDuration() < Settings::fTargetLockPOVHoldDuration) {
				//directionalMovementHandler->ToggleTargetLock(!directionalMovementHandler->HasTargetLocked());
				return;
			}
		}

		_ProcessButton(a_this, a_event, a_data);
	}
	
	void HorseCameraStateHook::OnEnterState(ConsoleRE::HorseCameraState* a_this)
	{
		_OnEnterState(a_this);

		if (DirectionalMovementHandler::GetSingleton()->GetFreeCameraEnabled()) {
			auto playerCharacter = ConsoleRE::PlayerCharacter::GetSingleton();

			ConsoleRE::Actor* horse = nullptr;
			horse = static_cast<ConsoleRE::Actor*>(a_this->horseRefHandle.get().get());

			if (savedCamera.rotationType != SaveCamera::RotationType::kNone) {
				auto rotationType = savedCamera.rotationType;
				ConsoleRE::NiPoint2 rot = savedCamera.ConsumeRotation();
				if (rotationType == SaveCamera::RotationType::kThirdPerson) {
					playerCharacter->data.angle.x = -rot.y;
				}
				
				a_this->freeRotation.x = NormalAbsoluteAngle(rot.x - horse->data.angle.z);
			}

			if (savedCamera.bZoomSaved) {
				float zoomOffset, pitchZoomOffset;
				savedCamera.ConsumeZoom(zoomOffset, pitchZoomOffset);
				a_this->targetZoomOffset = zoomOffset;
				a_this->currentZoomOffset = a_this->targetZoomOffset;
				a_this->savedZoomOffset = a_this->targetZoomOffset;
			}

			a_this->horseCurrentDirection = horse->GetHeading(false);
		}
	}

	void HorseCameraStateHook::OnExitState(ConsoleRE::HorseCameraState* a_this)
	{
		if (DirectionalMovementHandler::GetSingleton()->GetFreeCameraEnabled()) {
			savedCamera.SaveRotation(a_this->freeRotation, SaveCamera::RotationType::kHorse);
			savedCamera.SaveZoom(a_this->currentZoomOffset, a_this->pitchZoomOffset);
		}

		_OnExitState(a_this);
	}

	void HorseCameraStateHook::UpdateRotation(ConsoleRE::HorseCameraState* a_this)
	{
		auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();
		if (directionalMovementHandler->GetFreeCameraEnabled() && !directionalMovementHandler->IFPV_IsFirstPerson() && !directionalMovementHandler->ImprovedCamera_IsFirstPerson()) {
			float horseCurrentDirection = a_this->horseCurrentDirection;
			float freeRotationX = a_this->freeRotation.x;

			a_this->freeRotationEnabled = true;

			_UpdateRotation(a_this);

			a_this->horseCurrentDirection = horseCurrentDirection;
			a_this->freeRotation.x = freeRotationX;

			if (a_this->horseRefHandle) {
				ConsoleRE::Actor* horse = nullptr;
				horse = static_cast<ConsoleRE::Actor*>(a_this->horseRefHandle.get().get());
				if (horse) {
					float heading = horse->GetHeading(false);

					a_this->freeRotation.x += a_this->horseCurrentDirection - heading;

					NiQuaternion_SomeRotationManipulation(a_this->rotation, -a_this->freeRotation.y, 0.f, heading + a_this->freeRotation.x);
					a_this->horseCurrentDirection = heading;
				}
			}
		} else {
			_UpdateRotation(a_this);
		}
	}

	void HorseCameraStateHook::HandleLookInput(ConsoleRE::HorseCameraState* a_this, const ConsoleRE::NiPoint2& a_input)
	{
		if (DirectionalMovementHandler::GetSingleton()->HasTargetLocked()) {
			return;
		}

		_HandleLookInput(a_this, a_input);
	}

	void HorseCameraStateHook::ProcessButton(ConsoleRE::HorseCameraState* a_this, ConsoleRE::ButtonEvent* a_event, ConsoleRE::PlayerControlsData* a_data)
	{
		auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();
		if (a_event && directionalMovementHandler->HasTargetLocked() && Settings::bTargetLockUseScrollWheel) {
			auto& userEvent = a_event->QUserEvent();
			auto userEvents = ConsoleRE::UserEvents::GetSingleton();

			if (userEvent == userEvents->zoomIn) {
				directionalMovementHandler->SwitchTarget(DirectionalMovementHandler::Direction::kLeft);
				return;
			} else if (userEvent == userEvents->zoomOut) {
				directionalMovementHandler->SwitchTarget(DirectionalMovementHandler::Direction::kRight);
				return;
			}
		}

		if (a_event && BSInputDeviceManager_IsUsingGamepad(ConsoleRE::BSInputDeviceManager::GetSingleton()) ? Settings::bTargetLockUsePOVSwitchGamepad : Settings::bTargetLockUsePOVSwitchKeyboard) {
			auto& userEvent = a_event->QUserEvent();
			auto userEvents = ConsoleRE::UserEvents::GetSingleton();

			if (userEvent == userEvents->togglePOV && a_event->IsUp() && a_event->HeldDuration() < Settings::fTargetLockPOVHoldDuration) {
				//directionalMovementHandler->ToggleTargetLock(!directionalMovementHandler->HasTargetLocked());
				return;
			}
		}

		_ProcessButton(a_this, a_event, a_data);
	}
	
	void TweenMenuCameraStateHook::OnEnterState(ConsoleRE::TESCameraState* a_this)
	{
		if (DirectionalMovementHandler::GetSingleton()->IsFreeCamera()) {
			savedCamera.rotationType = SaveCamera::RotationType::kNone;
		}
		
		_OnEnterState(a_this);
	}

	void TweenMenuCameraStateHook::OnExitState(ConsoleRE::TESCameraState* a_this)
	{
		if (DirectionalMovementHandler::GetSingleton()->GetFreeCameraEnabled()) {
			savedCamera.rotationType = SaveCamera::RotationType::kNone;
		}

		_OnExitState(a_this);
	}

	void VATSCameraStateHook::OnExitState(ConsoleRE::TESCameraState* a_this)
	{
		if (DirectionalMovementHandler::GetSingleton()->GetFreeCameraEnabled()) {
			savedCamera.rotationType = SaveCamera::RotationType::kNone;
		}

		_OnExitState(a_this);
	}

	void PlayerCameraTransitionStateHook::OnEnterState(ConsoleRE::PlayerCameraTransitionState* a_this)
	{
		if (a_this->transitionFrom->id == ConsoleRE::CameraStates::kMount && a_this->transitionTo->id == ConsoleRE::CameraStates::kThirdPerson) {
			if (savedCamera.rotationType == SaveCamera::RotationType::kHorse) {
				auto thirdPersonState = static_cast<ConsoleRE::ThirdPersonState*>(a_this->transitionTo);
				auto playerCharacter = ConsoleRE::PlayerCharacter::GetSingleton();
				thirdPersonState->freeRotation.x = savedCamera.ConsumeYaw();
				playerCharacter->data.angle.x = -savedCamera.ConsumePitch();
			}
		} else if (a_this->transitionFrom->id == ConsoleRE::CameraStates::kMount && a_this->transitionTo->id == ConsoleRE::CameraStates::kFirstPerson) {
			if (savedCamera.rotationType == SaveCamera::RotationType::kHorse) {
				auto playerCharacter = ConsoleRE::PlayerCharacter::GetSingleton();
				playerCharacter->data.angle.x = -savedCamera.ConsumePitch();
			}
		}

		_OnEnterState(a_this);
	}

	void MovementHandlerAgentPlayerControlsHook::Func1(void* a1, void* a2)
	{
		// disable dampening controls while sprinting during lock-on, so you don't retain any weird momentum when rotating back to target after sprinting
		auto playerCharacter = ConsoleRE::PlayerCharacter::GetSingleton();
		if (DirectionalMovementHandler::GetSingleton()->HasTargetLocked() && playerCharacter && playerCharacter->AsActorState()->IsSprinting()) {
			*g_bDampenPlayerControls = false;
		} else {
			*g_bDampenPlayerControls = true;
		}
		_Func1(a1, a2);
	}

	void ProjectileHook::ProjectileAimSupport(ConsoleRE::Projectile* a_this)
	{
		auto projectileNode = a_this->Get3D2();

		// player only, 0x100000 == player
		auto& shooter = a_this->shooter;
		auto& desiredTarget = a_this->desiredTarget;
		if (projectileNode && shooter.native_handle() == 0x100000) {
			auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();
			if (directionalMovementHandler->HasTargetLocked() || desiredTarget.native_handle() != 0) {
				TargetLockProjectileAimType aimType;

				switch (static_cast<ConsoleRE::FormType>(a_this->formType))
				{
				case ConsoleRE::FormType::ProjectileArrow:
					aimType = Settings::uTargetLockArrowAimType;
					break;
				case ConsoleRE::FormType::ProjectileMissile:
					aimType = Settings::uTargetLockMissileAimType;
					break;
				default:
					aimType = TargetLockProjectileAimType::kFreeAim;
				}

				if (aimType != TargetLockProjectileAimType::kFreeAim) {
					if (!desiredTarget.get()) {
						auto target = directionalMovementHandler->GetTarget();
						auto targetPoint = directionalMovementHandler->GetTargetPoint();
						if (!target || !targetPoint) {
							return;
						}
						desiredTarget = target;
						directionalMovementHandler->AddProjectileTarget(a_this->GetHandle(), targetPoint);

						if (aimType == TargetLockProjectileAimType::kPredict) {
							// predict only at the start (desiredTarget not yet set), then let the projectile go unchanged in next updates
							
							if (desiredTarget) {
								ConsoleRE::NiPoint3 targetPos = targetPoint->world.translate;
								ConsoleRE::NiPoint3 targetVelocity;
								target.get()->GetLinearVelocity(targetVelocity);

								float projectileGravity = 0.f;
								if (auto ammo = a_this->ammoSource) {
									if (auto bgsProjectile = ammo->data.projectile) {
										projectileGravity = bgsProjectile->data.gravity;
										if (auto bhkWorld = a_this->parentCell->GetbhkWorld()) {
											if (auto hkpWorld = bhkWorld->GetWorld1()) {
												auto vec4 = hkpWorld->gravity;
												float quad[4];
												_mm_store_ps(quad, vec4.quad);
												float gravity = -quad[2] * *g_worldScaleInverse;
												projectileGravity *= gravity;
											}
										}
									}
								}

								auto& linearVelocity = a_this->linearVelocity;

								PredictAimProjectile(a_this->data.location, targetPos, targetVelocity, projectileGravity, linearVelocity);

								// rotate
								ConsoleRE::NiPoint3 direction = linearVelocity;
								direction.Unitize();

								a_this->data.angle.x = asin(direction.z);
								a_this->data.angle.z = atan2(direction.x, direction.y);

								if (a_this->data.angle.z < 0.0) {
									a_this->data.angle.z += PI;
								}

								if (direction.x < 0.0) {
									a_this->data.angle.z += PI;
								}

								SetRotationMatrix(projectileNode->local.rotate, -direction.x, direction.y, direction.z);
							}
						}
					}

					
					if (aimType == TargetLockProjectileAimType::kHoming) {
						// homing
						auto targetPoint = directionalMovementHandler->GetProjectileTargetPoint(a_this->GetHandle());
						if (targetPoint) {
							ConsoleRE::NiPoint3 targetPos = targetPoint->world.translate;
							auto& linearVelocity = a_this->linearVelocity;
							float speed = linearVelocity.Length();

							//if (speed < 1500.f) {
							//	return _GetLinearVelocity(a_this, a_outVelocity);
							//}

							ConsoleRE::NiPoint3 direction = (targetPos - a_this->data.location);

							// normalize direction
							direction.Unitize();

							// rotate
							a_this->data.angle.x = asin(direction.z);
							a_this->data.angle.z = atan2(direction.x, direction.y);

							if (a_this->data.angle.z < 0.0) {
								a_this->data.angle.z += PI;
							}

							if (direction.x < 0.0) {
								a_this->data.angle.z += PI;
							}

							SetRotationMatrix(projectileNode->local.rotate, -direction.x, direction.y, direction.z);
							linearVelocity = direction * speed;
						}
					}
				}
			}
		}
	}

	void ProjectileHook::GetLinearVelocityProjectile(ConsoleRE::Projectile* a_this, ConsoleRE::NiPoint3& a_outVelocity)
	{
		_GetLinearVelocityProjectile(a_this, a_outVelocity);

		ProjectileAimSupport(a_this);
	}

	void ProjectileHook::GetLinearVelocityArrow(ConsoleRE::Projectile* a_this, ConsoleRE::NiPoint3& a_outVelocity)
	{
		_GetLinearVelocityArrow(a_this, a_outVelocity);

		ProjectileAimSupport(a_this);
	}

	void ProjectileHook::GetLinearVelocityMissile(ConsoleRE::Projectile* a_this, ConsoleRE::NiPoint3& a_outVelocity)
	{
		_GetLinearVelocityMissile(a_this, a_outVelocity);

		ProjectileAimSupport(a_this);
	}

	bool ProjectileHook::Func183(ConsoleRE::Projectile* a_this)
	{
		// player only, 0x100000 == player
		auto& shooter = a_this->shooter;
		auto& desiredTarget = a_this->desiredTarget;
		if (shooter.native_handle() == 0x100000) {
			auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();
			if (directionalMovementHandler->HasTargetLocked() &&
					desiredTarget.native_handle() == 0 &&                                          // the chained projectiles from spells like chain lightning will have the desiredTarget  
					Settings::uTargetLockMissileAimType != TargetLockProjectileAimType::kFreeAim) {        // handle variable filled by the aim support feature, the parent one doesn't yet.
				auto beamProjectile = skyrim_cast<ConsoleRE::BeamProjectile*>(a_this);
				auto target = directionalMovementHandler->GetTarget();
				auto targetPoint = directionalMovementHandler->GetTargetPoint();
				if (beamProjectile && target && targetPoint) {
					desiredTarget = target;
					directionalMovementHandler->AddProjectileTarget(a_this->GetHandle(), targetPoint);

					ConsoleRE::NiPoint3 targetPos = targetPoint->world.translate;
					ConsoleRE::NiPoint3 direction = (targetPos - a_this->data.location);

					// normalize direction
					direction.Unitize();

					// rotate
					a_this->data.angle.x = atan2(-direction.z, std::sqrtf(direction.x * direction.x + direction.y * direction.y));
					a_this->data.angle.z = atan2(direction.x, direction.y);

					if (a_this->data.angle.z < 0.0) {
						a_this->data.angle.z += PI;
					}

					if (direction.x < 0.0) {
						a_this->data.angle.z += PI;
					}
				}
			}
		}

		// call the original vfunc
		return a_this->RunTargetPick();
	}

	static Raycast::RayHitCollector collector;

	void ProjectileHook::InitProjectile(ConsoleRE::Projectile* a_this)
	{
		_InitProjectile(a_this);

		auto& shooter = a_this->shooter;
		if (shooter.native_handle() == 0x100000) {
			auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();
			if (!directionalMovementHandler->HasTargetLocked() && directionalMovementHandler->GetCurrentlyMountedAiming()) 
			{
				auto playerCamera = ConsoleRE::PlayerCamera::GetSingleton();
				if (playerCamera->currentState && playerCamera->currentState->id == ConsoleRE::CameraState::kMount) 
				{
					auto horseCameraState = static_cast<ConsoleRE::HorseCameraState*>(playerCamera->currentState.get());
					const ConsoleRE::NiPoint3 forwardVector{ 0.f, 1.f, 0.f };
					const ConsoleRE::NiPoint3 upVector{ 0.f, 0.f, 1.f };
					ConsoleRE::NiQuaternion cameraRotation;
					horseCameraState->GetRotation(cameraRotation);
					auto cameraForwardVector = RotateVector(forwardVector, cameraRotation);

					cameraForwardVector.Unitize();

					auto cameraPos = playerCamera->cameraRoot->world.translate;

					ConsoleRE::NiPoint3 rayStart = cameraPos;
					ConsoleRE::NiPoint3 rayEnd = cameraPos + cameraForwardVector * 5000.f;
					ConsoleRE::NiPoint3 hitPos = rayEnd;

					ConsoleRE::NiPoint3 cameraToPlayer = playerCamera->cameraTarget.get()->GetPosition() - cameraPos;
					ConsoleRE::NiPoint3 cameraToTarget = rayEnd - cameraPos;
					ConsoleRE::NiPoint3 projected = Project(cameraToPlayer, cameraToTarget);
					ConsoleRE::NiPoint3 projectedPos = ConsoleRE::NiPoint3(projected.x + cameraPos.x, projected.y + cameraPos.y, projected.z + cameraPos.z);
					rayStart = projectedPos;

					uint16_t playerCollisionGroup = 0;

					auto playerCharacter = ConsoleRE::PlayerCharacter::GetSingleton();
					if (auto playerBody = playerCharacter->Get3D()) {
						if (auto collisionObject = playerBody->GetCollisionObject()) {
							if (auto rigidBody = collisionObject->GetRigidBody()) {
								playerCollisionGroup = static_cast<ConsoleRE::hkpEntity*>(rigidBody->referencedObject.get())->collidable.broadPhaseHandle.collisionFilterInfo >> 16;
							}
						}
					}

					float bhkWorldScale = *g_worldScale;

					collector.Reset();
					ConsoleRE::hkpWorldRayCastInput raycastInput;
					raycastInput.filterInfo = ((uint32_t)playerCollisionGroup << 16) | 0x28;
					raycastInput.from.quad = _mm_setr_ps(rayStart.x * bhkWorldScale, rayStart.y * bhkWorldScale, rayStart.z * bhkWorldScale, 0.f);
					raycastInput.to.quad = _mm_setr_ps(rayEnd.x * bhkWorldScale, rayEnd.y * bhkWorldScale, rayEnd.z * bhkWorldScale, 0.f);
					auto world = playerCharacter->parentCell->GetbhkWorld();
					world->worldLock.LockForRead();
					CastRay(world->GetWorld2(), raycastInput, collector);
					world->worldLock.UnlockForRead();
					if (collector.doesHitExist) {
						auto distance = rayEnd - rayStart;
						hitPos = rayStart + (distance * collector.closestHitInfo.hitFraction);
					}

					//bool bHit = collector.doesHitExist;
									
					ConsoleRE::NiPoint3 direction = hitPos - a_this->data.location;
					direction.Unitize();

					float rotationX, rotationZ;

					rotationX = atan2(-direction.z, std::sqrtf(direction.x * direction.x + direction.y * direction.y));
					rotationZ = atan2(direction.x, direction.y);

					if (rotationZ < 0.0) {
						rotationZ += PI;
					}

					if (direction.x < 0.0) {
						rotationZ += PI;
					}

					a_this->data.angle.x = rotationX;
					a_this->data.angle.z = rotationZ;

					auto projectileNode = a_this->Get3D();
					if (projectileNode) {
						SetRotationMatrix(projectileNode->local.rotate, cameraForwardVector.x, cameraForwardVector.y, cameraForwardVector.z);
					}

					auto& linearVelocity = a_this->linearVelocity;
					float velocityScalar = linearVelocity.Length();

					ConsoleRE::NiPoint3 rightVector = direction.Cross(upVector);
					direction = RotateAngleAxis(direction, AngleToRadian(*g_f3PArrowTiltUpAngle), rightVector);

					linearVelocity = direction * velocityScalar;
				}
			}
		}
	}

	void CharacterHook::Update(ConsoleRE::Actor* a_this, float a_delta)
	{
		_Update(a_this, a_delta);

		DirectionalMovementHandler::GetSingleton()->UpdateLeaning(a_this, a_delta);
	}

	void PlayerCharacterHook::UpdateAnimation(ConsoleRE::Actor* a_this, float a_delta)
	{
		auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();
		auto previousAimAngle = directionalMovementHandler->GetCurrentHorseAimAngle();
		directionalMovementHandler->SetCurrentHorseAimAngle(0.f); // the horse aim function only gets called when you're actually aiming on a horse, so we have to reset here to detect when you stop aiming
		directionalMovementHandler->SetCurrentlyMountedAiming(false);

		_UpdateAnimation(a_this, a_delta);

		auto updatedAimAngle = directionalMovementHandler->GetCurrentHorseAimAngle(); // if we were aiming, it will be updated by now

		if (updatedAimAngle != previousAimAngle) {
			directionalMovementHandler->SetPreviousHorseAimAngle(previousAimAngle);
			directionalMovementHandler->UpdateHorseAimDirection();
		}

		directionalMovementHandler->UpdateLeaning(a_this, a_delta);
	}

    void PlayerCharacterHook::Update(ConsoleRE::Actor* a_this, float a_delta)
	{
		_Update(a_this, a_delta);

		auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();
		if (directionalMovementHandler->IsICInstalled()) {
			if (auto playerCamera = ConsoleRE::PlayerCamera::GetSingleton()) {
				auto& currentCameraState = playerCamera->currentState;
				auto bIsInFirstPersonState = currentCameraState && currentCameraState->id == ConsoleRE::CameraState::kFirstPerson;
				if (bIsInFirstPersonState) {
					if (auto currentProcess = a_this->currentProcess) {
						AIProcess_ClearHeadTrackTarget(currentProcess);
					}
				}
			}
		}
	}

    void PlayerCharacterHook::ProcessTracking(ConsoleRE::Actor* a_this, float a_delta, ConsoleRE::NiAVObject* a_obj3D)
	{
		using HeadTrackType = ConsoleRE::HighProcessData::HEAD_TRACK_TYPE;
		// Handle TDM headtracking stuff that needs to be done before calling the original

		auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();
		auto actorState = a_this->AsActorState();

		bool bBehaviorPatchInstalled = DirectionalMovementHandler::IsBehaviorPatchInstalled(a_this);

		if (bBehaviorPatchInstalled)
		{
			// for good measure
			a_this->SetGraphVariableBool("IsNPC", false);
		}

		if (directionalMovementHandler->IFPV_IsFirstPerson() || directionalMovementHandler->ImprovedCamera_IsFirstPerson())
		{
			actorState->actorState2.headTracking = false;
			a_this->SetGraphVariableBool("IsNPC", false);
			return _ProcessTracking(a_this, a_delta, a_obj3D);
		}
		
		bool bIsHeadtrackingEnabled = Settings::bHeadtracking && !directionalMovementHandler->GetForceDisableHeadtracking();
		bool bIsBlocking = false;
		bool bIsSprinting = false;

		auto currentProcess = a_this->currentProcess;
		if (bIsHeadtrackingEnabled && currentProcess) {
			if (!bBehaviorPatchInstalled) {
				actorState->actorState2.headTracking = true;
				a_this->SetGraphVariableBool("IsNPC", true);
			}

			a_this->SetGraphVariableBool("bHeadTrackSpine", Settings::bHeadtrackSpine ? true : false);			
			
			// expire dialogue headtrack if timer is up
			if (currentProcess->high && currentProcess->high->headTracked[HeadTrackType::kCombat]) {
				if (directionalMovementHandler->GetDialogueHeadtrackTimer() <= 0.f) {
					currentProcess->high->SetHeadtrackTarget(HeadTrackType::kCombat, nullptr);
				}
			}

			// set headtracking variables if we have any target set
			auto target = currentProcess->GetHeadtrackTarget();
			if (target) {
				actorState->actorState2.headTracking = true;
				if (!bBehaviorPatchInstalled) {
					a_this->SetGraphVariableBool("IsNPC", true);
				}
			} else {
				actorState->actorState2.headTracking = false;
				/*if (!bBehaviorPatchInstalled) {
					a_this->SetGraphVariableBool("IsNPC", false);
				}	*/
			}

			bIsBlocking = actorState->actorState2.wantBlocking;
			// disable headtracking while attacking or blocking without behavior patch
			if ((!bBehaviorPatchInstalled && bIsBlocking) ||
				(actorState->actorState1.meleeAttackState > ConsoleRE::ATTACK_STATE_ENUM::kNone)) {
				actorState->actorState2.headTracking = false;
				if (!bBehaviorPatchInstalled) {
					a_this->SetGraphVariableBool("IsNPC", false);
				}
			}
		}

		ConsoleRE::NiPoint3 targetPos;

		//a_this->actorState2.headTracking = true;

		// reset headtrack
		if (bIsHeadtrackingEnabled && currentProcess && currentProcess->high) 
		{
			// clear the 0 and 1 targets if they're set for whatever reason
			auto selfHandle = a_this->GetHandle();
			if (currentProcess->high->headTracked[HeadTrackType::kDefault]) 
			{
				currentProcess->high->SetHeadtrackTarget(HeadTrackType::kDefault, nullptr);
			}
			if (currentProcess->high->headTracked[HeadTrackType::kAction]) 
			{
				currentProcess->high->SetHeadtrackTarget(HeadTrackType::kAction, nullptr);
			}
		}

		// run original function
		// If player is on a mount, need to force the headtracking target to forward direction because for some reason it gets stuck otherwise
		if (a_this->IsOnMount()) {
			targetPos = a_this->GetLookingAtLocation();
			float yaw = a_this->data.angle.z - PI2;
			float pitch = a_this->data.angle.x * 0.5f;

			auto currentState = ConsoleRE::PlayerCamera::GetSingleton()->currentState;
			if (currentState && currentState->id == ConsoleRE::CameraState::kMount) {
				auto horseCameraState = static_cast<ConsoleRE::HorseCameraState*>(currentState.get());
				yaw += horseCameraState->freeRotation.x;
			}

			yaw = NormalRelativeAngle(yaw);

			ConsoleRE::NiPoint3 offset = -RotationToDirection(yaw, pitch) * 500.f;
			offset.x *= -1.f;
			targetPos += offset;
			currentProcess->high->headTrackTargetOffset = targetPos;
			currentProcess->SetHeadtrackTarget(a_this, targetPos);
		}

		_ProcessTracking(a_this, a_delta, a_obj3D);

		// handle fake IsNPC
		if (!bIsHeadtrackingEnabled && bBehaviorPatchInstalled && !DirectionalMovementHandler::GetSingleton()->GetPlayerIsNPC()) {
			//a_this->currentProcess->SetHeadtrackTarget(a_this, targetPos);
			actorState->actorState2.headTracking = false;
		}

		bIsSprinting = actorState->actorState1.sprinting;

		if (bIsHeadtrackingEnabled &&
			Settings::bCameraHeadtracking &&
			(Settings::fCameraHeadtrackingDuration == 0.f || directionalMovementHandler->GetCameraHeadtrackTimer() > 0.f) &&
			!bIsSprinting &&
			!bIsBlocking &&
			currentProcess &&
			(a_this->boolBits & ConsoleRE::Actor::BOOL_BITS::kHasSceneExtra) == 0) 
		{
			// try camera headtracking
			auto highProcess = currentProcess->high;
			if (highProcess && 
				actorState->actorState1.meleeAttackState == ConsoleRE::ATTACK_STATE_ENUM::kNone &&
				actorState->actorState1.sitSleepState != ConsoleRE::SIT_SLEEP_STATE::kIsSleeping &&
				!highProcess->headTracked[HeadTrackType::kScript] && !highProcess->headTracked[HeadTrackType::kCombat] && !highProcess->headTracked[HeadTrackType::kDialogue] && !highProcess->headTracked[HeadTrackType::kProcedure])
			{
				if (!bBehaviorPatchInstalled) {
					actorState->actorState2.headTracking = true;
					a_this->SetGraphVariableBool("IsNPC", true);
				}

				directionalMovementHandler->UpdateCameraHeadtracking();
			}
		}

		// force look at target point
		if (bIsHeadtrackingEnabled && currentProcess && currentProcess->high) {
			if (currentProcess->high->headTracked[HeadTrackType::kDialogue] && !currentProcess->high->headTracked[HeadTrackType::kProcedure] && actorState->actorState1.meleeAttackState == ConsoleRE::ATTACK_STATE_ENUM::kNone) {
				if (auto targetPoint = directionalMovementHandler->GetTargetPoint()) {
					currentProcess->SetHeadtrackTarget(a_this, targetPoint->world.translate);
				}
			}
		}
	}

	static void ApplyYawDelta(ConsoleRE::ActorState* a_actorState, ConsoleRE::NiPoint3& a_angle)
	{
		auto actor = SKSE::stl::adjust_pointer<ConsoleRE::Actor>(a_actorState, -0xB8);
		auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();
		
		bool bIsAIDriven = actor->movementController && !actor->movementController->unk1C5;
		if (!bIsAIDriven) {
			a_angle.z -= DirectionalMovementHandler::GetSingleton()->GetYawDelta();
		} else {
			directionalMovementHandler->ResetYawDelta();
		}
	}

	void PlayerCharacterHook::GetAngle(ConsoleRE::ActorState* a_this, ConsoleRE::NiPoint3& a_angle)
	{
		_GetAngle(a_this, a_angle);
		
		ApplyYawDelta(a_this, a_angle);
	}

	void PlayerCharacterHook::UpdateSprintState(ConsoleRE::PlayerCharacter* a_this)
	{
		auto playerControls = ConsoleRE::PlayerControls::GetSingleton();
		ConsoleRE::NiPointer<ConsoleRE::Actor> mount = nullptr;
		bool bMounted = a_this->GetMount(mount);
		auto actor = bMounted ? mount.get() : a_this;
		if (a_this != nullptr) {
			bool bShouldBeSprinting = false;

			auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();
			bool bShouldFaceCrosshair = directionalMovementHandler->ShouldFaceCrosshair();

			ConsoleRE::NiPoint2 normalizedInputDirection = directionalMovementHandler->GetActualInputDirection();
			normalizedInputDirection.Unitize();

			bool bAutoMove = playerControls->data.autoMove;

			auto actorState = actor->AsActorState();
			
			bool bIsSyncSprintState = Actor_IsSyncSprintState(actor);
			bool bIsSprintingRunningOrBlocking = actorState->actorState1.sprinting == true || actor->IsRunning() || actor->IsBlocking();
			bool bUnk1 = Actor_CanSprint_CheckCharacterControllerValues(actor);
			bool bIsOverEncumbered = actor->IsOverEncumbered();
			bool bUnk2 = Actor_IsSyncSprintState(a_this) || (actorState->GetAttackState() == ConsoleRE::ATTACK_STATE_ENUM::kNone);
			bool bIsPreviousMoveInputForward = ((bShouldFaceCrosshair && !bAutoMove) ? normalizedInputDirection.y : playerControls->data.prevMoveVec.y) > 0.f;
			bool bIsNotStrafing = bShouldFaceCrosshair ? 0.75f > fabs(normalizedInputDirection.x) : *g_fSprintStopThreshold > fabs(playerControls->data.prevMoveVec.x);
			bool bIsStaminaNotZero = actor->AsActorValueOwner()->GetActorValue(ConsoleRE::ActorValue::kStamina) > 0.f;
			bool bIsSprinting = a_this->playerFlags.isSprinting;

			// added
			
			bool bHasMovementInput = playerControls->data.prevMoveVec.x != 0.f || playerControls->data.prevMoveVec.y != 0.f;
			//bool bIsAttacking = directionalMovementHandler->GetAttackState() != DirectionalMovementHandler::AttackState::kNone;
			bool bIsDodging = directionalMovementHandler->IsDodging();
			//int iState;
			//a_this->GetGraphVariableInt("iState", iState);
			//bool bIsCasting = iState == 10;
			bool bFreeCamTargetLocked = directionalMovementHandler->HasTargetLocked() && directionalMovementHandler->IsFreeCamera();
			
			if (bMounted)
			{
				bIsPreviousMoveInputForward = bHasMovementInput;
				bIsNotStrafing = bHasMovementInput;
			}

			bool bSpecific = bFreeCamTargetLocked ? bHasMovementInput && !bIsDodging : bIsSprintingRunningOrBlocking && bIsPreviousMoveInputForward && bIsNotStrafing;  // branch depending on the mode we're in

			if (bIsSprinting &&
				!bUnk1 &&
				!bIsOverEncumbered &&
				bUnk2 &&
				bIsStaminaNotZero &&
				bSpecific)
			{
				bShouldBeSprinting = true;
			} else {
				bShouldBeSprinting = false;
				a_this->playerFlags.isSprinting = false;
			}

			if (bIsSyncSprintState != bShouldBeSprinting) {
				PlayerControls_CalledWhenSprintStateChanges(playerControls, 66 - bShouldBeSprinting, 2);  // ?
			}
		}
	}

	//void AIProcess_SetRotationSpeedZHook::AIProcess_SetRotationSpeedZ(ConsoleRE::AIProcess* a_this, float a_rotationSpeed)
	//{
	//	if (a_this) {
	//		if (ConsoleRE::PlayerCharacter::GetSingleton()->currentProcess == a_this && DirectionalMovementHandler::GetSingleton()->IsFreeCamera()) {
	//			return;	 // skip because we're setting it elsewhere and it'd overwrite to 0
	//		}
	//		a_this->middleHigh->rotationSpeed.z = a_rotationSpeed;
	//	}
	//}

	void AIProcess_SetRotationSpeedZHook::AIProcess_SetRotationSpeedZ1(ConsoleRE::AIProcess* a_this, float a_rotationSpeed)
	{
		if (a_this == ConsoleRE::PlayerCharacter::GetSingleton()->currentProcess && DirectionalMovementHandler::GetSingleton()->IsFreeCamera()) {
			return; // skip because we're setting it elsewhere and it'd overwrite to 0
		}
		return _AIProcess_SetRotationSpeedZ1(a_this, a_rotationSpeed);
	}

	void AIProcess_SetRotationSpeedZHook::AIProcess_SetRotationSpeedZ2(ConsoleRE::AIProcess* a_this, float a_rotationSpeed)
	{
		if (a_this == ConsoleRE::PlayerCharacter::GetSingleton()->currentProcess && DirectionalMovementHandler::GetSingleton()->IsFreeCamera()) {
			return; // skip because we're setting it elsewhere and it'd overwrite to 0
		}
		return _AIProcess_SetRotationSpeedZ2(a_this, a_rotationSpeed);
	}

	void AIProcess_SetRotationSpeedZHook::AIProcess_SetRotationSpeedZ3(ConsoleRE::AIProcess* a_this, float a_rotationSpeed)
	{
		if (a_this == ConsoleRE::PlayerCharacter::GetSingleton()->currentProcess && DirectionalMovementHandler::GetSingleton()->IsFreeCamera()) {
			return; // skip because we're setting it elsewhere and it'd overwrite to 0
		}
		return _AIProcess_SetRotationSpeedZ3(a_this, a_rotationSpeed);
	}

	void Actor_SetRotationHook::Actor_SetRotationX(ConsoleRE::Actor* a_this, float a_angle)
	{
		if (a_this->IsPlayerRef()) {
			auto thirdPersonState = static_cast<ConsoleRE::ThirdPersonState*>(ConsoleRE::PlayerCamera::GetSingleton()->cameraStates[ConsoleRE::CameraState::kThirdPerson].get());
			if (ConsoleRE::PlayerCamera::GetSingleton()->currentState.get() == thirdPersonState && thirdPersonState->freeRotationEnabled) {
				float angleDelta = a_angle - a_this->data.angle.x;
				thirdPersonState->freeRotation.y += angleDelta;
			}
		}

		_Actor_SetRotationX(a_this, a_angle);
	}

	void Actor_SetRotationHook::Actor_SetRotationZ1(ConsoleRE::Actor* a_this, float a_angle)
	{
		if (a_this->IsPlayerRef())
		{
			auto thirdPersonState = static_cast<ConsoleRE::ThirdPersonState*>(ConsoleRE::PlayerCamera::GetSingleton()->cameraStates[ConsoleRE::CameraState::kThirdPerson].get());
			if (ConsoleRE::PlayerCamera::GetSingleton()->currentState.get() == thirdPersonState && thirdPersonState->freeRotationEnabled) {
				float angleDelta = a_angle - a_this->data.angle.z;
				thirdPersonState->freeRotation.x -= angleDelta;
			}
		}

		_Actor_SetRotationZ1(a_this, a_angle);
	}

	void Actor_SetRotationHook::Actor_SetRotationZ2(ConsoleRE::Actor* a_this, float a_angle)
	{
		if (a_this->IsPlayerRef()) {
			auto thirdPersonState = static_cast<ConsoleRE::ThirdPersonState*>(ConsoleRE::PlayerCamera::GetSingleton()->cameraStates[ConsoleRE::CameraState::kThirdPerson].get());
			if (ConsoleRE::PlayerCamera::GetSingleton()->currentState.get() == thirdPersonState && thirdPersonState->freeRotationEnabled) {
				float angleDelta = a_angle - a_this->data.angle.z;
				thirdPersonState->freeRotation.x -= angleDelta;
			}
		}

		_Actor_SetRotationZ2(a_this, a_angle);
	}

	bool EnemyHealthHook::ProcessMessage(uintptr_t a_enemyHealth, ConsoleRE::HUDData* a_hudData)
	{
		bool bReturn = _ProcessMessage(a_enemyHealth, a_hudData);

		auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();

		if (!directionalMovementHandler->IsMagnetismActive()) {
			ConsoleRE::ActorHandle actorHandle = *(ConsoleRE::ActorHandle*)(a_enemyHealth + 0x28);

			if (actorHandle) {
				DirectionalMovementHandler::GetSingleton()->SetSoftTarget(actorHandle);
			} else {
				DirectionalMovementHandler::GetSingleton()->SetSoftTarget(ConsoleRE::ActorHandle());
			}
		}

		return bReturn;
	}

	void HeadtrackingHook::SetHeadtrackTarget0(ConsoleRE::AIProcess* a_this, ConsoleRE::Actor* a_target)
	{	
		// Skip for player so we don't get random headtracking targets
		if (Settings::bHeadtracking && !DirectionalMovementHandler::GetSingleton()->GetForceDisableHeadtracking() && a_this == ConsoleRE::PlayerCharacter::GetSingleton()->currentProcess) {
			_SetHeadtrackTarget0(a_this, nullptr);
			return;
		}
		_SetHeadtrackTarget0(a_this, a_target);
	}

	void SetHeadtrackTarget4(ConsoleRE::AIProcess* a_this, ConsoleRE::Actor* a_target)
	{
		if (Settings::bHeadtracking && !DirectionalMovementHandler::GetSingleton()->GetForceDisableHeadtracking() && a_target && a_target->IsPlayerRef() && a_this->middleHigh) 
		{
			if (auto actor = a_this->GetUserData()) 
			{
				//_SetHeadtrackTarget4(a_target->currentProcess, actor);
				auto targetCurrentProcess = a_target->currentProcess;
				if (targetCurrentProcess && targetCurrentProcess->high) 
				{
					targetCurrentProcess->high->SetHeadtrackTarget(ConsoleRE::HighProcessData::HEAD_TRACK_TYPE::kCombat, actor);  // for player, use lower priority so target lock overrides dialogue targets
					DirectionalMovementHandler::GetSingleton()->RefreshDialogueHeadtrackTimer();
				}
			}
		}
	}

	void HeadtrackingHook::SetHeadtrackTarget4A(ConsoleRE::AIProcess* a_this, ConsoleRE::Actor* a_target)
	{
		_SetHeadtrackTarget4A(a_this, a_target);

		SetHeadtrackTarget4(a_this, a_target);
	}

	void HeadtrackingHook::SetHeadtrackTarget4B(ConsoleRE::AIProcess* a_this, ConsoleRE::Actor* a_target)
	{
		_SetHeadtrackTarget4B(a_this, a_target);

		SetHeadtrackTarget4(a_this, a_target);
	}

	void HeadtrackingHook::SetHeadtrackTarget4C(ConsoleRE::AIProcess* a_this, ConsoleRE::Actor* a_target)
	{
		_SetHeadtrackTarget4C(a_this, a_target);

		SetHeadtrackTarget4(a_this, a_target);
	}

	void HeadtrackingHook::SetHeadtrackTarget4D(ConsoleRE::AIProcess* a_this, ConsoleRE::Actor* a_target)
	{
		_SetHeadtrackTarget4D(a_this, a_target);

		SetHeadtrackTarget4(a_this, a_target);
	}

	static ConsoleRE::InputEvent* lastInputEvent = nullptr;

	bool PlayerControlsHook::Handle(ConsoleRE::PlayerControls* a_this, ConsoleRE::InputEvent** a_event)
	{
		DirectionalMovementHandler::ResetControls();
		lastInputEvent = *a_event;
		return _Handle(a_this, a_event);
	}

	void ApplyCameraMovement()
	{
		if (lastInputEvent) {
			auto playerControls = ConsoleRE::PlayerControls::GetSingleton();
			for (ConsoleRE::InputEvent* inputEvent = lastInputEvent; inputEvent != nullptr; inputEvent = inputEvent->next) {
				if (inputEvent->eventType == ConsoleRE::INPUT_EVENT_TYPE::kMouseMove) {
					ConsoleRE::MouseMoveEvent* mouseMoveEvent = static_cast<ConsoleRE::MouseMoveEvent*>(inputEvent);
					playerControls->lookHandler->ProcessMouseMove(mouseMoveEvent, &playerControls->data);
					PlayerControls_ApplyLookSensitivitySettings(playerControls, &playerControls->data.lookInputVec);
				} else if (inputEvent->GetEventType() == ConsoleRE::INPUT_EVENT_TYPE::kThumbstick) {
					ConsoleRE::ThumbstickEvent* thumbstickEvent = static_cast<ConsoleRE::ThumbstickEvent*>(inputEvent);
					if (thumbstickEvent->IsRight()) {
						playerControls->lookHandler->ProcessThumbstick(thumbstickEvent, &playerControls->data);
						PlayerControls_ApplyLookSensitivitySettings(playerControls, &playerControls->data.lookInputVec);
					}
				}
			}
		}
	}

	bool PlayerControlsHook::CheckIsInSyncAnim(void* a_a1, void* a_a2)
	{
		bool bIsInSyncAnim = _CheckIsInSyncAnim(a_a1, a_a2);

		if (bIsInSyncAnim) {
			ApplyCameraMovement();
		}

		return bIsInSyncAnim;
	}

	bool PlayerControlsHook::Check2(ConsoleRE::PlayerCharacter* a_this)
	{
		bool bResult = _Check2(a_this);

		auto& currentCameraState = ConsoleRE::PlayerCamera::GetSingleton()->currentState;
		auto bIsInFurnitureState = currentCameraState && currentCameraState->id == ConsoleRE::CameraState::kFurniture;
		
		if (bResult || bIsInFurnitureState) {
			ApplyCameraMovement();
		}

		return bResult;
	}

	bool PlayerControlsHook::Check3(ConsoleRE::PlayerCharacter* a_this)
	{
		bool bResult = _Check3(a_this);

		if (bResult) {
			ApplyCameraMovement();
		}

		return bResult;
	}

	//bool PlayerControlsHook::CanProcessControls(ConsoleRE::PlayerControls* a_this, ConsoleRE::InputEvent** a_eventPtr)
	//{
	//	bool bCanProcessControls = _CanProcessControls(a_this, a_eventPtr);

	//	// process camera movement during locked controls
	//	if (!bCanProcessControls && !ConsoleRE::UI::GetSingleton()->GameIsPaused()) {
	//		for (ConsoleRE::InputEvent* inputEvent = *a_eventPtr; inputEvent != nullptr; inputEvent = inputEvent->next) {
	//			if (inputEvent->eventType == ConsoleRE::INPUT_EVENT_TYPE::kMouseMove) {
	//				ConsoleRE::MouseMoveEvent* mouseMoveEvent = static_cast<ConsoleRE::MouseMoveEvent*>(inputEvent);
	//				a_this->lookHandler->ProcessMouseMove(mouseMoveEvent, &a_this->data);
	//				PlayerControls_ApplyLookSensitivitySettings(a_this, &a_this->data.lookInputVec);
	//			} else if (inputEvent->GetEventType() == ConsoleRE::INPUT_EVENT_TYPE::kThumbstick) {
	//				ConsoleRE::ThumbstickEvent* thumbstickEvent = static_cast<ConsoleRE::ThumbstickEvent*>(inputEvent);
	//				if (thumbstickEvent->IsRight()) {
	//					a_this->lookHandler->ProcessThumbstick(thumbstickEvent, &a_this->data);
	//					PlayerControls_ApplyLookSensitivitySettings(a_this, &a_this->data.lookInputVec);
	//				}
	//			}
	//		}
	//	}

	//	return bCanProcessControls;
	//}

	void NukeSetIsNPCHook::SetBool(ConsoleRE::IAnimationGraphManagerHolder* a_this, ConsoleRE::BSFixedString* a_variableName, bool a_value)
	{
		if (a_variableName && !strcasecmp(a_variableName->c_str(), "IsNPC"))
		{
			auto ref = static_cast<ConsoleRE::TESObjectREFR*>(a_this);
			auto formID = ref->formID;
			if (formID == 0x14 && DirectionalMovementHandler::IsBehaviorPatchInstalled(ref)) // player
			{
				DirectionalMovementHandler::GetSingleton()->SetPlayerIsNPC(a_value);
				a_value = false;
			}
		}
		_SetBool(a_this, a_variableName, a_value);
	}

	void NukeSetIsNPCHook::SetInt(ConsoleRE::IAnimationGraphManagerHolder* a_this, ConsoleRE::BSFixedString* a_variableName, int32_t a_value)
	{
		if (a_variableName && !strcasecmp(a_variableName->c_str(), "IsNPC"))
		{
			auto ref = static_cast<ConsoleRE::TESObjectREFR*>(a_this);
			auto formID = ref->formID;
			if (formID == 0x14 && DirectionalMovementHandler::IsBehaviorPatchInstalled(ref))  // player
			{
				DirectionalMovementHandler::GetSingleton()->SetPlayerIsNPC(a_value);
				a_value = 0;
			}
		}
		_SetInt(a_this, a_variableName, a_value);
	}

	void PlayerCameraHook::Update(ConsoleRE::TESCamera* a_this)
	{
		_Update(a_this);

		DirectionalMovementHandler::GetSingleton()->UpdatePlayerPitch();
	}

	void PlayerCameraHook::SetCameraState(ConsoleRE::TESCamera* a_this, ConsoleRE::TESCameraState* a_newState)
	{
		if (a_this->currentState)
		{
			auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();
			directionalMovementHandler->SetCameraStateBeforeTween(a_this->currentState->id);
		}

		_SetCameraState(a_this, a_newState);
	}

	void MainUpdateHook::Nullsub()
	{
		_Nullsub();

		DirectionalMovementHandler::GetSingleton()->Update();
	}

	static float angleToTarget = 0.f;
	float* HorseAimHook::GetHorseCameraFreeRotationYaw(ConsoleRE::PlayerCamera* a_this)
	{
		auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();
		float* ret = nullptr;
		if (directionalMovementHandler->HasTargetLocked() && directionalMovementHandler->GetTarget())
		{
			auto playerCharacter = ConsoleRE::PlayerCharacter::GetSingleton();
			auto target = directionalMovementHandler->GetTarget();
			ConsoleRE::NiPoint2 playerPos;
			playerPos.x = playerCharacter->GetPositionX();
			playerPos.y = playerCharacter->GetPositionY();
			ConsoleRE::NiPoint2 targetPos;
			targetPos.x = target.get()->GetPositionX();
			targetPos.y = target.get()->GetPositionY();

			ConsoleRE::NiPoint2 directionToTarget = ConsoleRE::NiPoint2(-(targetPos.x - playerPos.x), targetPos.y - playerPos.y);
			directionToTarget.Unitize();

			ConsoleRE::NiPoint2 forwardVector(0.f, 1.f);
			ConsoleRE::NiPoint2 currentCharacterDirection = Vec2Rotate(forwardVector, playerCharacter->data.angle.z);

			angleToTarget = GetAngle(currentCharacterDirection, directionToTarget);

			ret = &angleToTarget;
		} else {
			ret = _GetHorseCameraFreeRotationYaw(a_this);
		}
		
		directionalMovementHandler->SetCurrentlyMountedAiming(true);
		directionalMovementHandler->SetCurrentHorseAimAngle(*ret);

		return ret;
	}

	/*void HorseAimHook::GetMovementAgentPosition(ConsoleRE::Actor* a_this, ConsoleRE::NiPoint3& a_pos)
	{
		_GetMovementAgentPosition(a_this, a_pos);

		a_pos.z += 100.f;
	}*/

	void HorseAimHook::Func(ConsoleRE::PlayerCamera* a_this)
	{	
		auto playerCharacter = ConsoleRE::PlayerCharacter::GetSingleton();
		if (DirectionalMovementHandler::GetSingleton()->HasTargetLocked() && playerCharacter && playerCharacter->IsOnMount()) {
			return;
		}

		_Func(a_this);
	}

	float HorseAimHook::GetYaw(ConsoleRE::Actor* a_this)
	{
		// originally it returns the value of AimHeadingCurrent graph variable minus the character yaw
		[[maybe_unused]] float original = _GetYaw(a_this);
		ConsoleRE::ActorPtr mount;
		float angleOffset = 0.f;
		if (a_this->GetMount(mount)) {
			angleOffset = mount->data.angle.z;
		}
		return DirectionalMovementHandler::GetSingleton()->GetCurrentHorseAimAngle() + angleOffset;
	}
}
