#include "Events.h"
#include "Settings.h"
#include "DirectionalMovementHandler.h"
#include "Offsets.h"

namespace Events
{
	InputEventHandler* InputEventHandler::GetSingleton()
	{
		static InputEventHandler singleton;
		return std::addressof(singleton);
	}

	void InputEventHandler::Register()
	{
		auto deviceManager = ConsoleRE::BSInputDeviceManager::GetSingleton();
		deviceManager->AddEventSink(InputEventHandler::GetSingleton());
		xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "Registered %s", typeid(ConsoleRE::InputEvent).name());
	}

	auto InputEventHandler::ProcessEvent(ConsoleRE::InputEvent* const* a_event, ConsoleRE::BSTEventSource<ConsoleRE::InputEvent*>*) -> EventResult
	{
		using EventType = ConsoleRE::INPUT_EVENT_TYPE;
		using DeviceType = ConsoleRE::INPUT_DEVICE;

		if (!a_event) 
		{
			return EventResult::kContinue;
		}

		for (auto event = *a_event; event; event = event->next) 
		{
			if (event->eventType != EventType::kButton) 
			{
				continue;
			}

			auto& userEvent = event->QUserEvent();
			auto userEvents = ConsoleRE::UserEvents::GetSingleton();

			auto button = static_cast<ConsoleRE::ButtonEvent*>(event);
			if (button->IsDown()) 
			{
				auto key = button->idCode;
				switch (button->device) 
				{
				case DeviceType::kMouse:
					key += kMouseOffset;
					break;
				case DeviceType::kKeyboard:
					key += kKeyboardOffset;
					break;
				case DeviceType::kGamepad:
					key = GetGamepadIndex((ConsoleRE::BSOrbisGamepadDevice::Key)key);
					break;
				default:
					continue;
				}

				auto ui = ConsoleRE::UI::GetSingleton();
				auto controlMap = ConsoleRE::ControlMap::GetSingleton();
				if (ui->GameIsPaused() || !controlMap->IsMovementControlsEnabled()) 
				{
					continue;
				}

				if (key == Settings::uTargetLockKey) 
				{
					bool bIgnore = false;

					if (userEvent == userEvents->togglePOV) 
					{
						switch (button->device) 
						{
						case DeviceType::kKeyboard:
							bIgnore = Settings::bTargetLockUsePOVSwitchKeyboard;
							break;
						case DeviceType::kGamepad:
							bIgnore = Settings::bTargetLockUsePOVSwitchGamepad;
							break;
						}
					}

					if (bIgnore) 
					{
						break;
					}

					auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();
					directionalMovementHandler->ToggleTargetLock(!directionalMovementHandler->HasTargetLocked(), true);
				}

				if (key == Settings::uSwitchTargetLeftKey) 
				{
					auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();
					if (directionalMovementHandler->HasTargetLocked() && !directionalMovementHandler->ShouldFaceCrosshair()) 
					{
						directionalMovementHandler->SwitchTarget(DirectionalMovementHandler::Direction::kLeft);
					}
				}

				if (key == Settings::uSwitchTargetRightKey) {
					auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();
					if (directionalMovementHandler->HasTargetLocked() && !directionalMovementHandler->ShouldFaceCrosshair())
					{
						directionalMovementHandler->SwitchTarget(DirectionalMovementHandler::Direction::kRight);
					}
				}
			}
						
			if (userEvent == userEvents->jump)
			{
				auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();
				if (button->IsHeld()) 
				{
					directionalMovementHandler->_pressedDirections.set(DirectionalMovementHandler::Direction::kUp);
				}
				else 
				{
					directionalMovementHandler->_pressedDirections.reset(DirectionalMovementHandler::Direction::kUp);
				}
			} 
			else if (userEvent == userEvents->sneak) 
			{
				auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();
				if (button->IsHeld()) 
				{
					directionalMovementHandler->_pressedDirections.set(DirectionalMovementHandler::Direction::kDown);
				} else 
				{
					directionalMovementHandler->_pressedDirections.reset(DirectionalMovementHandler::Direction::kDown);
				}
			}
		}

		return EventResult::kContinue;
	}

	uint32_t InputEventHandler::GetGamepadIndex(ConsoleRE::BSOrbisGamepadDevice::Key a_key)
	{
		return a_key;
	}

	CrosshairRefManager* CrosshairRefManager::GetSingleton()
	{
		static CrosshairRefManager singleton;
		return std::addressof(singleton);
	}

	void CrosshairRefManager::Register()
	{
		auto crosshair = API::GetCrosshairRefEventSource();
		crosshair->AddEventSink(CrosshairRefManager::GetSingleton());
		xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "Registered %s", typeid(SKSE::CrosshairRefEvent).name());
	}

	ConsoleRE::ObjectRefHandle CrosshairRefManager::GetCachedRef()
	{
		return _cachedRef;
	}

	EventResult CrosshairRefManager::ProcessEvent(const SKSE::CrosshairRefEvent* a_event, ConsoleRE::BSTEventSource<SKSE::CrosshairRefEvent>*)
	{
		_cachedRef = a_event && a_event->crosshairRef ? a_event->crosshairRef->CreateRefHandle() : ConsoleRE::ObjectRefHandle();

		return EventResult::kContinue;
	}

	EventHandler* EventHandler::GetSingleton()
	{
		static EventHandler singleton;
		return std::addressof(singleton);
	}

	void EventHandler::Register()
	{
		auto scriptEventSourceHolder = ConsoleRE::ScriptEventSourceHolder::GetSingleton();
		scriptEventSourceHolder->GetEventSource<ConsoleRE::TESDeathEvent>()->AddEventSink(EventHandler::GetSingleton());
		xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "Registered %s", typeid(ConsoleRE::TESDeathEvent).name());
		scriptEventSourceHolder->GetEventSource<ConsoleRE::TESEnterBleedoutEvent>()->AddEventSink(EventHandler::GetSingleton());
		xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "Registered %s", typeid(ConsoleRE::TESEnterBleedoutEvent).name());
	}

	// On death - toggle target lock
	EventResult EventHandler::ProcessEvent(const ConsoleRE::TESDeathEvent* a_event, ConsoleRE::BSTEventSource<ConsoleRE::TESDeathEvent>*)
	{
		auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();
		if (a_event && a_event->actorDying) {
			if (directionalMovementHandler->HasTargetLocked() && directionalMovementHandler->GetTarget() == a_event->actorDying->GetHandle()) {
				if (Settings::bAutoTargetNextOnDeath) {
					directionalMovementHandler->ToggleTargetLock(true);
				} else {
					directionalMovementHandler->ToggleTargetLock(false);
				}
			}
		}

		return EventResult::kContinue;
	}

	// On bleedout - for essential targets
	EventResult EventHandler::ProcessEvent(const ConsoleRE::TESEnterBleedoutEvent* a_event, ConsoleRE::BSTEventSource<ConsoleRE::TESEnterBleedoutEvent>*)
	{
		auto directionalMovementHandler = DirectionalMovementHandler::GetSingleton();
		if (a_event && a_event->actor) {
			auto actor = a_event->actor->As<ConsoleRE::Actor>();
			if (actor && actor->IsEssential())
			{
				if (directionalMovementHandler->HasTargetLocked() && directionalMovementHandler->GetTarget() == a_event->actor->GetHandle()) {
					if (Settings::bAutoTargetNextOnDeath) {
						directionalMovementHandler->ToggleTargetLock(true);
					} else {
						directionalMovementHandler->ToggleTargetLock(false);
					}
				}
			}
		}

		return EventResult::kContinue;
	}

	void SinkEventHandlers()
	{
		InputEventHandler::Register();
		CrosshairRefManager::Register();
		EventHandler::Register();

		xUtilty::Log::GetSingleton(0)->Write(xUtilty::Log::logLevel::kNone, "Registered all event handlers");
	}
}
