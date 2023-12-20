#pragma once
#include "TrueDirectionalMovementAPI.h"

class DirectionalMovementHandler;

namespace Messaging
{
	using APIResult = ::TDM_API::APIResult;
	using InterfaceVersion1 = ::TDM_API::IVTDM1;
	using InterfaceVersion2 = ::TDM_API::IVTDM2;

	class TDMInterface : public InterfaceVersion2
	{
	private:
		TDMInterface() noexcept;
		virtual ~TDMInterface() noexcept;

	public:
		static TDMInterface* GetSingleton() noexcept
		{
			static TDMInterface singleton;
			return std::addressof(singleton);
		}

		// InterfaceVersion1
		virtual unsigned long GetTDMThreadId() const noexcept override;
		virtual bool GetDirectionalMovementState() noexcept override;
		virtual bool GetTargetLockState() noexcept override;
		virtual ConsoleRE::ActorHandle GetCurrentTarget() noexcept override;
		virtual APIResult RequestDisableDirectionalMovement(size_t a_modHandle) noexcept override;
		virtual APIResult RequestDisableHeadtracking(size_t a_modHandle) noexcept override;
		virtual size_t GetDisableDirectionalMovementOwner() const noexcept override;
		virtual size_t GetDisableHeadtrackingOwner() const noexcept override;
		virtual APIResult ReleaseDisableDirectionalMovement(size_t a_modHandle) noexcept override;
		virtual APIResult ReleaseDisableHeadtracking(size_t a_modHandle) noexcept override;

		// InterfaceVersion2
		virtual APIResult RequestYawControl(size_t a_modHandle, float a_yawRotationSpeedMultiplier) noexcept override;
		virtual APIResult SetPlayerYaw(size_t a_modHandle, float a_desiredYaw) noexcept override;
		virtual APIResult ReleaseYawControl(size_t a_modHandle) noexcept override;

		// Internal
		// Mark directional movement control as required by True Directional Movement for API requests
		void SetNeedsDirectionalMovementControl(bool a_needsControl) noexcept;
		// Mark headtracking control as required by True Directional Movement for API requests
		void SetNeedsHeadtrackingControl(bool a_needsControl) noexcept;
		// Mark player yaw control as required by True Directional Movement for API requests
		void SetNeedsYawControl(bool a_needsControl) noexcept;

		// Does a mod have control over the directional movement?
		bool IsDirectionalMovementControlTaken() const noexcept;
		// Does a mod have control over the headtracking?
		bool IsHeadtrackingControlTaken() const noexcept;
		// Does a mod have control over the player character's yaw?
		bool IsYawControlTaken() const noexcept;

	private:
		unsigned long apiTID = 0;

		bool needsDirectionalMovementControl = false;
		std::atomic<size_t> directionalMovementOwner{ static_cast<size_t>(-1) };

		bool needsHeadtrackingControl = false;
		std::atomic<size_t> headtrackingOwner { static_cast<size_t>(-1)};

		bool needsYawControl = false;
		std::atomic<size_t> yawOwner{ static_cast<size_t>(-1) };
	};
}
