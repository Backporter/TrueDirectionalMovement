#pragma once

namespace Events
{
	using EventResult = ConsoleRE::BSEventNotifyControl;

	class InputEventHandler : public ConsoleRE::BSTEventSink<ConsoleRE::InputEvent*>
	{
	public:
		static InputEventHandler*	GetSingleton();
		static void					Register();

		virtual EventResult			ProcessEvent(ConsoleRE::InputEvent* const* a_event, ConsoleRE::BSTEventSource<ConsoleRE::InputEvent*>* a_eventSource) override;

	private:
		enum : uint32_t
		{
			kInvalid = static_cast<uint32_t>(-1),
			
			kKeyboardOffset = 0,
			kMouseOffset	= 0,
			kGamepadOffset	= 0,
		};

		InputEventHandler() = default;
		InputEventHandler(const InputEventHandler&) = delete;
		InputEventHandler(InputEventHandler&&) = delete;
		virtual ~InputEventHandler() = default;

		InputEventHandler& operator=(const InputEventHandler&) = delete;
		InputEventHandler& operator=(InputEventHandler&&) = delete;

		uint32_t GetGamepadIndex(ConsoleRE::BSOrbisGamepadDevice::Key a_key);
	};

	class CrosshairRefManager : public ConsoleRE::BSTEventSink<SKSE::CrosshairRefEvent>
	{
	public:
		static CrosshairRefManager*			GetSingleton();
		static void							Register();
		ConsoleRE::ObjectRefHandle			GetCachedRef();

		virtual EventResult					ProcessEvent(const SKSE::CrosshairRefEvent* a_event, ConsoleRE::BSTEventSource<SKSE::CrosshairRefEvent>* a_eventSource) override;

	private:
		CrosshairRefManager() = default;
		CrosshairRefManager(const CrosshairRefManager&) = delete;
		CrosshairRefManager(CrosshairRefManager&&) = delete;
		virtual ~CrosshairRefManager() = default;

		CrosshairRefManager& operator=(const CrosshairRefManager&) = delete;
		CrosshairRefManager& operator=(CrosshairRefManager&&) = delete;

		ConsoleRE::ObjectRefHandle _cachedRef;

	};

	class EventHandler : 
		public ConsoleRE::BSTEventSink<ConsoleRE::TESDeathEvent>,
		public ConsoleRE::BSTEventSink<ConsoleRE::TESEnterBleedoutEvent>
	{
	public:
		static EventHandler* GetSingleton();
		static void Register();

		virtual EventResult ProcessEvent(const ConsoleRE::TESDeathEvent* a_event, ConsoleRE::BSTEventSource<ConsoleRE::TESDeathEvent>* a_eventSource) override;
		virtual EventResult ProcessEvent(const ConsoleRE::TESEnterBleedoutEvent* a_event, ConsoleRE::BSTEventSource<ConsoleRE::TESEnterBleedoutEvent>* a_eventSource) override;

	private:
		EventHandler() = default;
		EventHandler(const EventHandler&) = delete;
		EventHandler(EventHandler&&) = delete;
		virtual ~EventHandler() = default;

		EventHandler& operator=(const EventHandler&) = delete;
		EventHandler& operator=(EventHandler&&) = delete;
	};

	void SinkEventHandlers();
}
