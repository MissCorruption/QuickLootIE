#pragma once

namespace Input
{
	constexpr inline auto QUICKLOOT_FLAG = static_cast<RE::UserEvents::USER_EVENT_FLAG>(1 << 12);

	class ControlGroups
	{
	public:
		enum Group
		{
			kPageKeys,
			kArrowKeys,
			kMouseWheel,
			kDPAD,
			kTotal
		};

		[[nodiscard]] static ControlGroups& get() noexcept
		{
			static ControlGroups singleton;
			return singleton;
		}

		[[nodiscard]] constexpr bool& operator[](Group a_group) noexcept { return _enabled[a_group]; }
		[[nodiscard]] constexpr bool operator[](Group a_group) const noexcept { return _enabled[a_group]; }

	private:
		ControlGroups() = default;
		ControlGroups(const ControlGroups&) = delete;
		ControlGroups(ControlGroups&&) = delete;

		~ControlGroups() = default;

		ControlGroups& operator=(const ControlGroups&) = delete;
		ControlGroups& operator=(ControlGroups&&) = delete;

		static inline std::array<bool, kTotal> _enabled{ false };
	};
	using Group = ControlGroups::Group;

	class InputManager
	{
	public:
		static void Install()
		{
			if (REL::Module::IsAE()) {
				constexpr std::array locations{
					std::make_pair<std::uint64_t, std::size_t>(53270, 0x17),
					std::make_pair<std::uint64_t, std::size_t>(53299, 0x17),
					std::make_pair<std::uint64_t, std::size_t>(68534, 0x165),
					std::make_pair<std::uint64_t, std::size_t>(68540, 0x266),
				};

				auto& trampoline = SKSE::GetTrampoline();
				for (const auto& [id, offset] : locations) {
					REL::Relocation<std::uintptr_t> target(REL::ID(id), offset);
					_RefreshLinkedMappings = trampoline.write_call<5>(target.address(), RefreshLinkedMappings);
				}
			} else {
				constexpr std::array locations{
					std::make_pair<std::uint64_t, std::size_t>(52374, 0x17),
					std::make_pair<std::uint64_t, std::size_t>(52400, 0x17),
					std::make_pair<std::uint64_t, std::size_t>(67234, 0x113),
					std::make_pair<std::uint64_t, std::size_t>(67240, 0x17B),
				};

				auto& trampoline = SKSE::GetTrampoline();
				for (const auto& [id, offset] : locations) {
					REL::Relocation<std::uintptr_t> target(REL::ID(id), offset);
					_RefreshLinkedMappings = trampoline.write_call<5>(target.address(), RefreshLinkedMappings);
				}
			}
		}

	private:
		class UserEventMap
		{
		public:
			using value_type = std::string_view;

			UserEventMap() :
				_mappings{}
			{
				insert("Activate"sv);
				insert("Ready Weapon"sv);
			}

			void operator()(std::size_t a_device, RE::ControlMap::UserEventMapping& a_userEvent) const
			{
				using UEFlag = RE::UserEvents::USER_EVENT_FLAG;

				const auto& mapping = _mappings[a_device];
				auto it = mapping.find(a_userEvent.eventID);
				if (it != mapping.end()) {
					if (a_userEvent.userEventGroupFlag.all(UEFlag::kInvalid)) {
						a_userEvent.userEventGroupFlag = UEFlag::kNone;
					}

					a_userEvent.userEventGroupFlag.set(QUICKLOOT_FLAG);
				}
			}

		private:
			void insert(value_type a_value)
			{
				for (std::size_t i = 0; i < RE::INPUT_DEVICES::kTotal; ++i) {
					_mappings[i].insert(a_value);
				}
			}

			struct cicompare
			{
				[[nodiscard]] bool operator()(const value_type& a_lhs, const value_type& a_rhs) const
				{
					return _stricmp(a_lhs.data(), a_rhs.data()) < 0;
				}
			};

			std::array<std::set<value_type, cicompare>, RE::INPUT_DEVICES::kTotal> _mappings;
		};

		class IDCodeMap
		{
		public:
			using value_type = std::uint32_t;

			IDCodeMap()
			{
				using Device = RE::INPUT_DEVICE;
				using Gamepad = RE::BSWin32GamepadDevice::Key;
				using Keyboard = RE::BSWin32KeyboardDevice::Key;
				using Mouse = RE::BSWin32MouseDevice::Key;

				insert<OptionalGroup>(Group::kPageKeys, Device::kKeyboard, { Keyboard::kPageUp, Keyboard::kPageDown });
				insert<OptionalGroup>(Group::kArrowKeys, Device::kKeyboard, { Keyboard::kUp, Keyboard::kDown, Keyboard::kLeft, Keyboard::kRight });

				insert<MandatoryGroup>(Group::kMouseWheel, Device::kMouse, { Mouse::kWheelUp, Mouse::kWheelDown });
				insert<MandatoryGroup>(Group::kDPAD, Device::kGamepad, { Gamepad::kUp, Gamepad::kDown, Gamepad::kLeft, Gamepad::kRight });
			}

			void operator()(std::size_t a_device, RE::ControlMap::UserEventMapping& a_userEvent) const
			{
				const auto& mapping = _mappings[a_device];
				auto it = mapping.find(a_userEvent.inputKey);
				if (it != mapping.end()) {
					it->second->accept(a_userEvent);
				}
			}

			void commit()
			{
				for (const auto& mapping : _mappings) {
					for (const auto& [id, group] : mapping) {
						group->commit();
					}
				}
			}

		private:
			class IControlGroup
			{
			public:
				using value_type = RE::ControlMap::UserEventMapping;

				IControlGroup(Group a_group) noexcept :
					_group(a_group)
				{}

				virtual ~IControlGroup() = default;

				void accept(value_type& a_mapping)
				{
					if (_good) {
						if (can_accept(a_mapping)) {
							_queued.emplace_back(a_mapping);
						} else {
							_good = false;
						}
					}
				}

				void commit() noexcept
				{
					using UEFlag = RE::UserEvents::USER_EVENT_FLAG;

					if (_good) {
						for (auto& todo : _queued) {
							auto& mapping = todo.get();
							if (mapping.userEventGroupFlag.all(UEFlag::kInvalid)) {
								mapping.userEventGroupFlag = UEFlag::kNone;
							}

							mapping.userEventGroupFlag.set(QUICKLOOT_FLAG);
						}
					}

					ControlGroups::get()[_group] = _good;
				}

			protected:
				[[nodiscard]] virtual bool can_accept(const value_type& a_mapping) const noexcept = 0;

			private:
				std::vector<std::reference_wrapper<value_type>> _queued;
				Group _group;
				bool _good{ true };
			};

			class MandatoryGroup final :
				public IControlGroup
			{
			private:
				using super = IControlGroup;

			public:
				using value_type = typename super::value_type;

				using super::super;
				using super::operator=;

			protected:
				[[nodiscard]] bool can_accept(const value_type&) const noexcept override { return true; }
			};

			class OptionalGroup final :
				public IControlGroup
			{
			private:
				using super = IControlGroup;

			public:
				using value_type = typename super::value_type;

				using super::super;
				using super::operator=;

			protected:
				[[nodiscard]] bool can_accept(const value_type& a_mapping) const noexcept override { return !a_mapping.linked; }
			};

			template <
				class T,
				std::enable_if_t<
					std::is_base_of_v<IControlGroup, T>,
					int> = 0>
			void insert(Group a_group, RE::INPUT_DEVICE a_device, std::initializer_list<value_type> a_idCodes)
			{
				const auto group = std::make_shared<T>(a_group);
				for (const auto& idCode : a_idCodes) {
					_mappings[a_device].emplace(idCode, group);
				}
			}

			std::array<std::map<value_type, std::shared_ptr<IControlGroup>>, RE::INPUT_DEVICES::kTotal> _mappings;
		};

		InputManager() = delete;
		InputManager(const InputManager&) = delete;
		InputManager(InputManager&&) = delete;

		~InputManager() = delete;

		InputManager& operator=(const InputManager&) = delete;
		InputManager& operator=(InputManager&&) = delete;

		static void RefreshLinkedMappings(RE::ControlMap* a_controlMap)
		{
			_RefreshLinkedMappings(a_controlMap);
			if (!a_controlMap) {
				return;
			}

			const auto for_each = [&](std::function<void(RE::ControlMap::UserEventMapping&, std::size_t)> a_functor) {
				for (auto& map : a_controlMap->controlMap) {
					if (map) {
						for (std::size_t i = 0; i < RE::ControlMap::InputContext::GetNumDeviceMappings(); ++i) {
							for (auto& userMapping : map->deviceMappings[i]) {
								a_functor(userMapping, i);
							}
						}
					}
				}
			};

			for_each([=](RE::ControlMap::UserEventMapping& a_mapping, std::size_t) {
				if (a_mapping.userEventGroupFlag.none(RE::UserEvents::USER_EVENT_FLAG::kInvalid)) {
					a_mapping.userEventGroupFlag.reset(QUICKLOOT_FLAG);
				}
			});

			UserEventMap eventMap;
			IDCodeMap idMap;

			for_each([&](RE::ControlMap::UserEventMapping& a_mapping, std::size_t a_device) {
				eventMap(a_device, a_mapping);
				idMap(a_device, a_mapping);
			});

			idMap.commit();
			a_controlMap->ToggleControls(QUICKLOOT_FLAG, true);
		}

		inline static REL::Relocation<decltype(RefreshLinkedMappings)> _RefreshLinkedMappings;
	};

	class ControlMap
	{
	public:
		[[nodiscard]] std::uint32_t operator()(std::string_view a_userEvent) const
		{
			auto input = RE::BSInputDeviceManager::GetSingleton();
			if (!input) {
				return INVALID;
			}

			if (input->IsGamepadEnabled()) {
				return MapGamepad(a_userEvent);
			} else {
				auto result = MapKeyboard(a_userEvent);
				return result != INVALID ? result : MapMouse(a_userEvent);
			}
		}

	private:
		static constexpr std::uint32_t INVALID = 282;  // ???

		template <class T, std::size_t N>
		struct simple_array
		{
			T c[N];
		};

		[[nodiscard]] std::uint32_t MapGamepad(std::string_view a_userEvent) const
		{
			using Key = RE::BSWin32GamepadDevice::Keys;

			auto controlMap = RE::ControlMap::GetSingleton();
			if (!controlMap) {
				return INVALID;
			}

			constexpr auto mappings = []() noexcept {
				const auto map = [](std::size_t a_key) noexcept {
					for (std::size_t i = 0; i < 32; ++i) {
						auto to = (a_key >> i) & 1;
						if (to != 0) {
							return i;
						}
					}
					return static_cast<std::size_t>(0);
				};

				const auto assign = [](auto&& a_elem, auto&& a_first, auto&& a_second) noexcept {
					a_elem.first = static_cast<std::uint32_t>(a_first);
					a_elem.second = static_cast<std::uint32_t>(a_second);
				};

				constexpr auto N = []() noexcept {
					std::size_t size = 0;
					for (std::size_t i = Key::kRightShoulder; i != 0; i >>= 1) {
						++size;
					}
					return size + 4 + 2;
				}();

				std::array<std::pair<std::uint32_t, std::uint32_t>, N> arr{};
				std::size_t idx = 0;
				std::size_t frame = 266;
				for (std::size_t key = Key::kUp; key <= Key::kRightShoulder; key <<= 1) {
					assign(arr[idx++], key, map(key) + frame);
				}
				for (std::size_t key = Key::kA; key <= Key::kY; key <<= 1) {
					assign(arr[idx++], key, map(key >> 2) + frame);
				}

				assign(arr[idx], Key::kLeftTrigger, frame + idx);
				++idx;
				assign(arr[idx], Key::kRightTrigger, frame + idx);
				return frozen::make_map(arr);
			}();

			auto key = controlMap->GetMappedKey(a_userEvent, RE::INPUT_DEVICE::kGamepad);
			auto it = mappings.find(key);
			return it != mappings.end() ? it->second : INVALID;
		}

		[[nodiscard]] std::uint32_t MapKeyboard(std::string_view a_userEvent) const
		{
			using Key = RE::BSKeyboardDevice::Keys;

			auto controlMap = RE::ControlMap::GetSingleton();
			if (!controlMap) {
				return INVALID;
			}

			constexpr auto mappings = []() noexcept {
				const auto validate_range = [](auto&& mappings, std::uint32_t a_begin, std::uint32_t a_end) noexcept {
					for (auto i = a_begin; i <= a_end; ++i) {
						mappings[i] = i;
					}
				};

				std::array<std::uint32_t, Key::kDelete + 1> arr{ INVALID };
				validate_range(arr, Key::kEscape, Key::kKP_Decimal);
				validate_range(arr, Key::kF11, Key::kF12);
				validate_range(arr, Key::kKP_Enter, Key::kRightControl);
				validate_range(arr, Key::kKP_Divide, Key::kKP_Divide);
				validate_range(arr, Key::kPrintScreen, Key::kRightAlt);
				validate_range(arr, Key::kPause, Key::kPause);
				validate_range(arr, Key::kHome, Key::kPageUp);
				validate_range(arr, Key::kLeft, Key::kLeft);
				validate_range(arr, Key::kRight, Key::kRight);
				validate_range(arr, Key::kEnd, Key::kDelete);

				return arr;
			}();

			auto key = controlMap->GetMappedKey(a_userEvent, RE::INPUT_DEVICE::kKeyboard);
			return key <= mappings.size() ? mappings[key] : INVALID;
		}

		[[nodiscard]] std::uint32_t MapMouse(std::string_view a_userEvent) const
		{
			using Key = RE::BSWin32MouseDevice::Keys;

			auto controlMap = RE::ControlMap::GetSingleton();
			if (!controlMap) {
				return INVALID;
			}

			constexpr auto mappings = []() noexcept {
				std::array<std::pair<std::uint32_t, std::uint32_t>, Key::kWheelDown + 1> arr{};
				for (std::uint32_t i = Key::kLeftButton; i <= Key::kWheelDown; ++i) {
					arr[i].first = i;
					arr[i].second = i + 256;
				}

				return arr;
			}();

			auto key = controlMap->GetMappedKey(a_userEvent, RE::INPUT_DEVICE::kMouse);
			return key <= mappings.size() ? mappings[key].second : INVALID;
		}
	};
}
