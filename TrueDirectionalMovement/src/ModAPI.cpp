#include "ModAPI.h"
#include "DirectionalMovementHandler.h"

Messaging::TDMInterface::TDMInterface() noexcept
{
	// apiTID = GetCurrentThreadId();
}

Messaging::TDMInterface::~TDMInterface() noexcept {}

unsigned long Messaging::TDMInterface::GetTDMThreadId() const noexcept
{
	return apiTID;
}

bool Messaging::TDMInterface::GetDirectionalMovementState() noexcept
{
	auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();
	if (directionalMovementHandler) {
		return directionalMovementHandler->IsFreeCamera();
	}
	return false;
}

bool Messaging::TDMInterface::GetTargetLockState() noexcept
{
	auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();
	if (directionalMovementHandler) {
		return directionalMovementHandler->HasTargetLocked();
	}
	return false;
}

ConsoleRE::ActorHandle Messaging::TDMInterface::GetCurrentTarget() noexcept
{
	auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();
	if (directionalMovementHandler) {
		return directionalMovementHandler->GetTarget();
	}
	return ConsoleRE::ActorHandle();
}

Messaging::APIResult Messaging::TDMInterface::RequestDisableDirectionalMovement(size_t a_modHandle) noexcept
{
	const auto owner = directionalMovementOwner.load(std::memory_order::memory_order_acquire);
	if (owner != static_cast<size_t>(-1))
		if (owner == a_modHandle)
			return APIResult::AlreadyGiven;
		else
			return APIResult::AlreadyTaken;

	if (needsDirectionalMovementControl)
		return APIResult::MustKeep;
	auto expected = static_cast<size_t>(static_cast<size_t>(-1));
	if (!directionalMovementOwner.compare_exchange_strong(expected, a_modHandle, std::memory_order::memory_order_acq_rel))
		return APIResult::AlreadyTaken;

	auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();
	if (directionalMovementHandler) {
		directionalMovementHandler->SetForceDisableDirectionalMovement(true);
	}

	return APIResult::OK;
}

Messaging::APIResult Messaging::TDMInterface::RequestDisableHeadtracking(size_t a_modHandle) noexcept
{
	const auto owner = headtrackingOwner.load(std::memory_order::memory_order_acquire);
	if (owner != static_cast<size_t>(-1))
		if (owner == a_modHandle)
			return APIResult::AlreadyGiven;
		else
			return APIResult::AlreadyTaken;

	if (needsHeadtrackingControl)
		return APIResult::MustKeep;
	auto expected = static_cast<size_t>(static_cast<size_t>(-1));
	if (!headtrackingOwner.compare_exchange_strong(expected, a_modHandle, std::memory_order::memory_order_acq_rel))
		return APIResult::AlreadyTaken;

	auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();
	if (directionalMovementHandler) {
		directionalMovementHandler->SetForceDisableHeadtracking(true);
	}

	return APIResult::OK;
}

size_t Messaging::TDMInterface::GetDisableDirectionalMovementOwner() const noexcept
{
	return directionalMovementOwner;
}

size_t Messaging::TDMInterface::GetDisableHeadtrackingOwner() const noexcept
{
	return headtrackingOwner;
}

Messaging::APIResult Messaging::TDMInterface::ReleaseDisableDirectionalMovement(size_t a_modHandle) noexcept
{
	if (directionalMovementOwner != a_modHandle)
		return APIResult::NotOwner;
	directionalMovementOwner.store(static_cast<size_t>(-1), std::memory_order::memory_order_release);

	auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();
	if (directionalMovementHandler) {
		directionalMovementHandler->SetForceDisableDirectionalMovement(false);
	}

	return APIResult::OK;
}

Messaging::APIResult Messaging::TDMInterface::ReleaseDisableHeadtracking(size_t a_modHandle) noexcept
{
	if (headtrackingOwner != a_modHandle)
		return APIResult::NotOwner;
	headtrackingOwner.store(static_cast<size_t>(-1), std::memory_order::memory_order_release);

	auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();
	if (directionalMovementHandler) {
		directionalMovementHandler->SetForceDisableHeadtracking(false);
	}

	return APIResult::OK;
}

Messaging::APIResult Messaging::TDMInterface::RequestYawControl(size_t a_modHandle, float a_yawRotationSpeedMultiplier) noexcept
{
	const auto owner = yawOwner.load(std::memory_order::memory_order_acquire);
	if (owner != static_cast<size_t>(-1))
		if (owner == a_modHandle)
			return APIResult::AlreadyGiven;
		else
			return APIResult::AlreadyTaken;

	if (needsYawControl)
		return APIResult::MustKeep;
	auto expected = static_cast<size_t>(static_cast<size_t>(-1));
	if (!yawOwner.compare_exchange_strong(expected, a_modHandle, std::memory_order::memory_order_acq_rel))
		return APIResult::AlreadyTaken;

	auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();
	if (directionalMovementHandler) {
		directionalMovementHandler->SetYawControl(true, a_yawRotationSpeedMultiplier);
	}

	return APIResult::OK;
}

Messaging::APIResult Messaging::TDMInterface::SetPlayerYaw(size_t a_modHandle, float a_desiredYaw) noexcept
{
	if (yawOwner != a_modHandle)
		return APIResult::NotOwner;

	auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();
	if (directionalMovementHandler) {
		directionalMovementHandler->SetPlayerYaw(a_desiredYaw);
	}

	return APIResult::OK;
}

Messaging::APIResult Messaging::TDMInterface::ReleaseYawControl(size_t a_modHandle) noexcept
{
	if (yawOwner != a_modHandle)
		return APIResult::NotOwner;
	yawOwner.store(static_cast<size_t>(-1), std::memory_order::memory_order_release);

	auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();
	if (directionalMovementHandler) {
		directionalMovementHandler->SetYawControl(false);
	}

	return APIResult::OK;
}

void Messaging::TDMInterface::SetNeedsDirectionalMovementControl(bool a_needsControl) noexcept
{
	needsDirectionalMovementControl = a_needsControl;
}

void Messaging::TDMInterface::SetNeedsHeadtrackingControl(bool a_needsControl) noexcept
{
	needsHeadtrackingControl = a_needsControl;
}

void Messaging::TDMInterface::SetNeedsYawControl(bool a_needsControl) noexcept
{
	needsYawControl = a_needsControl;
}

bool Messaging::TDMInterface::IsDirectionalMovementControlTaken() const noexcept
{
	return directionalMovementOwner.load(std::memory_order::memory_order_acquire) != static_cast<size_t>(-1);
}

bool Messaging::TDMInterface::IsHeadtrackingControlTaken() const noexcept
{
	return headtrackingOwner.load(std::memory_order::memory_order_acquire) != static_cast<size_t>(-1);
}

bool Messaging::TDMInterface::IsYawControlTaken() const noexcept
{
	return yawOwner.load(std::memory_order::memory_order_acquire) != static_cast<size_t>(-1);
}
