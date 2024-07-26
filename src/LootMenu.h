#pragma once

#include "CLIK/Array.h"
#include "CLIK/GFx/Controls/ButtonBar.h"
#include "CLIK/GFx/Controls/ScrollingList.h"
#include "CLIK/TextField.h"
#include "ContainerChangedHandler.h"
#include "Integrations/APIServer.h"
#include "Items/OldGroundItems.h"
#include "Items/OldInventoryItem.h"
#include "Items/OldItem.h"
#include "OpenCloseHandler.h"
#include "ViewHandler.h"

namespace Scaleform
{
	class LootMenu :
		public RE::IMenu
	{
	private:
		using super = RE::IMenu;

		static inline RE::ObjectRefHandle _lastContainer{};
		static inline ptrdiff_t _lastSelectedIndex = 0;
		static inline bool _restoreLastSelectedIndex = false;

	public:
		static constexpr std::string_view MenuName() noexcept { return MENU_NAME; }
		static constexpr std::int8_t SortPriority() noexcept { return SORT_PRIORITY; }

		static void Register()
		{
			auto ui = RE::UI::GetSingleton();
			if (ui) {
				ui->Register(MENU_NAME, Creator);
				logger::info("Registered {}"sv, MENU_NAME);
			}
		}

		void ModSelectedIndex(double a_mod)
		{
			const auto maxIdx = static_cast<double>(_itemListImpl.size()) - 1.0;
			if (maxIdx >= 0.0) {
				auto idx = _itemList.SelectedIndex();
				idx += a_mod;
				idx = std::clamp(idx, 0.0, maxIdx);
				_lastSelectedIndex = static_cast<ptrdiff_t>(idx);
				_itemList.SelectedIndex(idx);
				UpdateInfoBar();
				OnSelectedIndexChanged();
			}
		}

		void ModSelectedPage(double a_mod)
		{
			auto& inst = _itemList.GetInstance();
			std::array<RE::GFxValue, 1> args;
			args[0] = a_mod;
			[[maybe_unused]] const auto success =
				inst.Invoke("modSelectedPage", args);
			assert(success);
			UpdateInfoBar();
			OnSelectedIndexChanged();
		}

		void SetContainer(RE::ObjectRefHandle a_ref)
		{
			assert(a_ref);
			_src = a_ref;
			_viewHandler->SetSource(a_ref);
			_containerChangedHandler.SetContainer(a_ref);
			_openCloseHandler.SetSource(a_ref);
			_itemList.SelectedIndex(0);

			logger::trace("Looking at container {:#016x}. Last container was {:#016x}. Last index was {}.",
				reinterpret_cast<uintptr_t>(a_ref.get().get()), reinterpret_cast<uintptr_t>(_lastContainer.get().get()), _lastSelectedIndex);
			_restoreLastSelectedIndex = a_ref == _lastContainer;
			// quick hack to fix behavior if SetContainer is called twice in a row before the ui refresh goes through
			if (!_restoreLastSelectedIndex)
				_lastSelectedIndex = 0;
			_lastContainer = a_ref;

			QueueUIRefresh();
		}

		void RefreshInventory()
		{
			const auto idx = static_cast<std::ptrdiff_t>(_itemList.SelectedIndex());

			_itemListImpl.clear();
			auto src = _src.get();
			if (!src) {
				_itemListProvider.ClearElements();
				_itemList.Invalidate();
				_itemList.SelectedIndex(-1.0);
				return;
			}

			const auto stealing = WouldBeStealing();
			auto inv = src->GetInventory(CanDisplay);
			for (auto& [obj, data] : inv) {
				auto& [count, entry] = data;
				if (count > 0 && entry) {
					_itemListImpl.push_back(std::make_unique<QuickLoot::Items::OldInventoryItem>(count, stealing, std::move(entry), _src));
				}
			}

			auto dropped = src->GetDroppedInventory(CanDisplay);
			for (auto& [obj, data] : dropped) {
				auto& [count, items] = data;
				if (count > 0 && !items.empty()) {
					_itemListImpl.push_back(std::make_unique<QuickLoot::Items::OldGroundItems>(count, stealing, std::move(items)));
				}
			}

			if (Settings::CloseWhenEmpty() && _itemListImpl.empty()) {
				Close();
			} else {
				Sort();
				std::vector<QuickLoot::Element> elements;
				_itemListProvider.ClearElements();
				for (const auto& elem : _itemListImpl) {
					_itemListProvider.PushBack(elem->GFxValue(*_view));
					elem->FillElementsVector(&elements);
				}
				_itemList.InvalidateData();
				_rootObj.GetInstance().Invoke("refresh");

				if (_restoreLastSelectedIndex) {
					logger::trace("Looking at the same container as before. Restoring last selection index ({}).", _lastSelectedIndex);
					_restoreLastSelectedIndex = false;
					RestoreIndex(_lastSelectedIndex);
				} else {
					RestoreIndex(idx);
				}

				UpdateWeight();
				UpdateInfoBar();

				_rootObj.Visible(true);
				OnSelectedIndexChanged();

				QuickLoot::API::APIServer::DispatchInvalidateLootMenuEvent(elements, _src);
			}
		}

		void RefreshUI()
		{
			RefreshInventory();
			UpdateTitle();
			UpdateButtonBar();
		}

		void TakeStack()
		{
			auto dst = _dst.get();
			auto pos = static_cast<std::ptrdiff_t>(_itemList.SelectedIndex());
			if (dst && 0 <= pos && pos < std::ssize(_itemListImpl)) {
				_itemListImpl[static_cast<std::size_t>(pos)]->TakeAll(*dst);
				_openCloseHandler.Open();

				// Taken from WaterFox' fork of QuickLootEE
				// See: https://github.com/Eloquence4/QuickLootEE/blob/c93e56dcb7f0372a5ad7df4b22e118e37deeb286/src/Scaleform/LootMenu.h#L136-L162
				if (Settings::DispelInvisibility() && dst->AsMagicTarget()) {
					DispelEffectsWithArchetype(dst->AsMagicTarget(), RE::EffectArchetypes::ArchetypeID::kInvisibility, false);
				}
			}

			QueueInventoryRefresh();
		}

		// TODO: This code works, but could be improved by using TransferInventory
		// Albeit there's currently no noticable lag doing it this way.
		void TakeAll()
		{
			auto dst = _dst.get();
			if (dst) {
				for (std::size_t i = 0; i < _itemListImpl.size(); ++i) {
					_itemListImpl[i]->TakeAll(*dst);
				}
				_openCloseHandler.Open();

				if (Settings::DispelInvisibility() && dst->AsMagicTarget()) {
					DispelEffectsWithArchetype(dst->AsMagicTarget(), RE::EffectArchetypes::ArchetypeID::kInvisibility, false);
				}
			}

			QueueInventoryRefresh();
		}

		void DispelEffectsWithArchetype(RE::MagicTarget* a_target, RE::MagicTarget::Archetype a_type, bool a_force)
		{
			if (!a_target || !a_target->GetActiveEffectList()) {
				return;
			}

			for (auto* effect : *a_target->GetActiveEffectList()) {
				if (effect && effect->GetBaseObject() && effect->GetBaseObject()->HasArchetype(a_type)) {
					effect->Dispel(a_force);
				}
			}
		}

	protected:
		using UIResult = RE::UI_MESSAGE_RESULTS;

		LootMenu()
		{
			auto menu = static_cast<super*>(this);
			menu->depthPriority = -1;
			auto scaleformManager = RE::BSScaleformManager::GetSingleton();

			[[maybe_unused]] const auto success = scaleformManager->LoadMovie(menu, menu->uiMovie, FILE_NAME.data());
			assert(success);

			auto def = menu->uiMovie->GetMovieDef();
			if (def) {
				def->SetState(
					RE::GFxState::StateType::kLog,
					RE::make_gptr<Logger>().get());
			}

			_viewHandler.emplace(menu, _dst);
			_view = menu->uiMovie;
			_view->SetMouseCursorCount(0);  // disable input, we'll handle it ourselves
			InitExtensions();
		}

		LootMenu(const LootMenu&) = default;
		LootMenu(LootMenu&&) = default;

		~LootMenu() = default;

		LootMenu& operator=(const LootMenu&) = default;
		LootMenu& operator=(LootMenu&&) = default;

		static stl::owner<RE::IMenu*> Creator() { return new LootMenu(); }

		void OnSelectedIndexChanged()
		{
			const auto idx = static_cast<std::ptrdiff_t>(_itemList.SelectedIndex());
			if (0 <= idx && idx < std::ssize(_itemListImpl)) {
				auto dst = _dst.get();
				const auto& item = _itemListImpl[static_cast<std::size_t>(idx)];
				if (item && dst) {
					item->OnSelected(*dst);
				}
			}
		}

		// IMenu
		void PostCreate() override { OnOpen(); }

		UIResult ProcessMessage(RE::UIMessage& a_message) override
		{
			using Type = RE::UI_MESSAGE_TYPE;

			switch (*a_message.type) {
			case Type::kHide:
				OnClose();
				return UIResult::kHandled;
			default:
				return super::ProcessMessage(a_message);
			}
		}

		void AdvanceMovie(float a_interval, std::uint32_t a_currentTime) override
		{
			auto src = _src.get();
			if (!src || src->IsActivationBlocked()) {
				Close();
			}

			ProcessDelegate();
			super::AdvanceMovie(a_interval, a_currentTime);
		}

		void RefreshPlatform() override
		{
			UpdateButtonBar();
		}

	private:
		class Logger :
			public RE::GFxLog
		{
		public:
			void LogMessageVarg(LogMessageType, const char* a_fmt, std::va_list a_argList) override
			{
				std::string fmt(a_fmt ? a_fmt : "");
				while (!fmt.empty() && fmt.back() == '\n') {
					fmt.pop_back();
				}

				std::va_list args;
				va_copy(args, a_argList);
				std::vector<char> buf(static_cast<std::size_t>(std::vsnprintf(0, 0, fmt.c_str(), a_argList) + 1));
				std::vsnprintf(buf.data(), buf.size(), fmt.c_str(), args);
				va_end(args);

				logger::info("{}: {}"sv, LootMenu::MenuName(), buf.data());
			}
		};

		[[nodiscard]] static bool CanDisplay(const RE::TESBoundObject& a_object)
		{
			switch (a_object.GetFormType()) {
			case RE::FormType::Scroll:
			case RE::FormType::Armor:
			case RE::FormType::Book:
			case RE::FormType::Ingredient:
			case RE::FormType::Misc:
			case RE::FormType::Weapon:
			case RE::FormType::Ammo:
			case RE::FormType::KeyMaster:
			case RE::FormType::AlchemyItem:
			case RE::FormType::Note:
			case RE::FormType::SoulGem:
				break;
			case RE::FormType::Light:
				{
					auto& light = static_cast<const RE::TESObjectLIGH&>(a_object);
					if (!light.CanBeCarried()) {
						return false;
					}
				}
				break;
			default:
				return false;
			}

			if (!a_object.GetPlayable()) {
				return false;
			}

			auto name = a_object.GetName();
			if (!name || name[0] == '\0') {
				return false;
			}

			return true;
		}

		void Close();

		void InitExtensions()
		{
			const RE::GFxValue boolean{ true };
			[[maybe_unused]] bool success = false;

			success = _view->SetVariable("_global.gfxExtensions", boolean);
			assert(success);
			//success = _view->SetVariable("_global.noInvisibleAdvance", boolean);
			assert(success);

			InjectUtilsClass();
		}

		void OnClose() {
			QuickLoot::API::APIServer::DispatchCloseLootMenuEvent(_src);
			return; 
		}

		void OnOpen()
		{
			// fallback values are for older versions of LootMenu.swf

			using element_t = std::tuple<std::reference_wrapper<CLIK::Object>, std::string_view, std::string_view>;
			std::array objects{
				element_t{ std::ref(_rootObj), "_root.lootMenu"sv, "_root.rootObj"sv },
				element_t{ std::ref(_title), "_root.lootMenu.title"sv, "_root.rootObj.title"sv },
				element_t{ std::ref(_weight), "_root.lootMenu.weight"sv, "_root.rootObj.weightContainer.textField"sv },
				element_t{ std::ref(_itemList), "_root.lootMenu.itemList"sv, "_root.rootObj.itemList"sv },
				element_t{ std::ref(_infoBar), "_root.lootMenu.infoBar"sv, "_root.rootObj.infoBar"sv },
				element_t{ std::ref(_buttonBar), "_root.lootMenu.buttonBar"sv, "_root.rootObj.buttonBar"sv }
			};

			for (const auto& [object, path, fallback] : objects) {
				auto& instance = object.get().GetInstance();
				bool success = _view->GetVariable(std::addressof(instance), path.data());
				if (!success) {
					success = _view->GetVariable(std::addressof(instance), fallback.data());
				}
				if (!success || !instance.IsObject()) {
					logger::error("Failed to find variable {} ({}) in {}.swf", path, fallback, FILE_NAME);
				}
			}

			const RE::GFxValue settings = BuildSettingsObject();
			_rootObj.GetInstance().Invoke("init", nullptr, &settings, 1);
			_rootObj.Visible(false);

			_title.AutoSize(CLIK::Object{ "left" });
			_title.Visible(false);
			_weight.AutoSize(CLIK::Object{ "left" });
			_weight.Visible(false);

			_view->CreateArray(std::addressof(_itemListProvider));
			_itemList.DataProvider(CLIK::Array{ _itemListProvider });

			_view->CreateArray(std::addressof(_infoBarProvider));
			_infoBar.DataProvider(CLIK::Array{ _infoBarProvider });

			_view->CreateArray(std::addressof(_buttonBarProvider));
			_buttonBar.DataProvider(CLIK::Array{ _buttonBarProvider });

			ProcessDelegate();

			QuickLoot::API::APIServer::DispatchOpenLootMenuEvent(_src);
		}

		RE::GFxValue BuildSettingsObject() const
        {
			RE::GFxValue settings{};

			if (!_view) {
				return settings;
			}

			_view->CreateObject(&settings);

			settings.SetMember("minLines", Settings::GetMinLines());
			settings.SetMember("maxLines", Settings::GetMaxLines());

			settings.SetMember("offsetX", Settings::GetWindowX());
			settings.SetMember("offsetY", Settings::GetWindowY());
			settings.SetMember("scale", Settings::GetWindowScale());

			settings.SetMember("alphaNormal", Settings::GetNormalWindowTransparency());
			settings.SetMember("alphaEmpty", Settings::GetEmptyWindowTransparency());

			double anchorFractionX = 0;
			double anchorFractionY = 0;
			ResolveAnchorPoint(Settings::GetAnchorPoint(), anchorFractionX, anchorFractionY);

			settings.SetMember("anchorFractionX", anchorFractionX);
			settings.SetMember("anchorFractionY", anchorFractionY);

			return settings;
		}

		static void ResolveAnchorPoint(QuickLoot::AnchorPoint anchor, double& fractionX, double& fractionY)
		{
			switch (anchor) {
			case QuickLoot::kTopLeft:
			case QuickLoot::kCenterLeft:
			case QuickLoot::kBottomLeft:
				fractionX = 0.0;
				break;

			case QuickLoot::kTopCenter:
			case QuickLoot::kCenter:
			case QuickLoot::kBottomCenter:
				fractionX = 0.5;
				break;

			case QuickLoot::kTopRight:
			case QuickLoot::kCenterRight:
			case QuickLoot::kBottomRight:
				fractionX = 1.0;
				break;
			}

			switch (anchor) {
			case QuickLoot::kTopLeft:
			case QuickLoot::kTopCenter:
			case QuickLoot::kTopRight:
				fractionY = 0.0;
				break;

			case QuickLoot::kCenterLeft:
			case QuickLoot::kCenter:
			case QuickLoot::kCenterRight:
				fractionY = 0.5;
				break;

			case QuickLoot::kBottomLeft:
			case QuickLoot::kBottomCenter:
			case QuickLoot::kBottomRight:
				fractionY = 1.0;
				break;
			}
		}

		void ProcessDelegate();
		void QueueInventoryRefresh();
		void QueueUIRefresh();
		void UtilsLog(const RE::GFxFunctionHandler::Params& params);
		void InjectUtilsClass();

		void RestoreIndex(std::ptrdiff_t a_oldIdx)
		{
			logger::trace("Trying to restore selected index to {} ({} items)", static_cast<uint32_t>(a_oldIdx), static_cast<uint32_t>(std::ssize(_itemListImpl)));

			if (const auto ssize = std::ssize(_itemListImpl); 0 <= a_oldIdx && a_oldIdx < ssize) {
				_itemList.SelectedIndex(static_cast<double>(a_oldIdx));
			} else if (!_itemListImpl.empty()) {
				if (a_oldIdx >= ssize) {
					_itemList.SelectedIndex(static_cast<double>(ssize) - 1.0);
				} else {
					_itemList.SelectedIndex(0.0);
				}
			} else {
				_itemList.SelectedIndex(-1.0);
			}

			logger::trace("New selected index: {}", static_cast<uint32_t>(_itemList.SelectedIndex()));
		}

		void Sort()
		{
			std::ranges::stable_sort(_itemListImpl,
				[&](auto&& a_lhs, auto&& a_rhs) {
					uintptr_t lhs_addr = reinterpret_cast<std::uintptr_t>(a_lhs.get());
					uintptr_t rhs_addr = reinterpret_cast<std::uintptr_t>(a_rhs.get());

					if (lhs_addr == 0 || lhs_addr > 0xFFFFFFFFFFFF ||
						rhs_addr == 0 || rhs_addr > 0xFFFFFFFFFFFF) {
						logger::warn("Error: Invalid pointer address detected."sv);
						return false;
					}

					return *a_lhs < *a_rhs;
				});
		}

		void UpdateButtonBar()
		{
			if (!_view) {
				return;
			}

			struct ButtonDefinition
			{
				const char* labelKey;
				const char* labelFallback;

				const char* stealingLabelKey;
				const char* stealingLabelFallback;

				const char* keybindEvent;
			};

			const std::array buttonDefs{
				ButtonDefinition{ "sSearch", "Search", "sStealFrom", "Steal From", "Favorites" },
				ButtonDefinition{ "sTake", "Take", "sSteal", "Steal", "Activate" },
				ButtonDefinition{ "sTakeAll", "Take All", "sTakeAll", "Take All", "Ready Weapon" },
			};

			const bool stealing = WouldBeStealing();

			_buttonBarProvider.ClearElements();
			
			for (std::size_t i = 0; i < buttonDefs.size(); ++i) {
				const auto& button = buttonDefs[i];

				const auto labelKey = stealing ? button.stealingLabelKey : button.labelKey;
				const auto labelFallback = stealing ? button.stealingLabelFallback : button.labelFallback;

				const auto setting = RE::GameSettingCollection::GetSingleton()->GetSetting(labelKey);
				const auto label = setting ? setting->GetString() : labelFallback;
				const auto index = Input::ControlMap()(button.keybindEvent);

				RE::GFxValue obj;
				_view->CreateObject(&obj);

				obj.SetMember("label", label);
				obj.SetMember("index", index);
				obj.SetMember("stolen", stealing);

				_buttonBarProvider.PushBack(obj);
			}
			_buttonBar.InvalidateData();
		}

		void UpdateInfoBar()
		{
			_infoBarProvider.ClearElements();
			const auto idx = static_cast<std::ptrdiff_t>(_itemList.SelectedIndex());
			if (0 <= idx && idx < std::ssize(_itemListImpl)) {
				typedef std::function<std::string(const QuickLoot::Items::OldItem&)> functor;
				const std::array functors{
					functor{ [](const QuickLoot::Items::OldItem& a_val) { return fmt::format(FMT_STRING("{:.1f}"), a_val.Weight()); } },
					functor{ [](const QuickLoot::Items::OldItem& a_val) { return fmt::format(FMT_STRING("{}"), a_val.Value()); } },
				};

				const auto& item = _itemListImpl[static_cast<std::size_t>(idx)];
				std::string str;
				RE::GFxValue obj;
				for (const auto& functor : functors) {
					str = functor(*item);
					obj.SetString(str);
					_infoBarProvider.PushBack(obj);
				}

				const auto ench = item->EnchantmentCharge();
				if (ench >= 0.0) {
					str = fmt::format(FMT_STRING("{:.1f}%"), ench);
					obj.SetString(str);
					_infoBarProvider.PushBack(obj);
				}
			}

			_infoBar.InvalidateData();
		}

		void UpdateTitle()
		{
			auto src = _src.get();
			if (src) {
				_title.HTMLText(
					stl::safe_string(
						src->GetDisplayFullName()));
				_title.Visible(true);
			}
		}

		void UpdateWeight()
		{
			auto dst = _dst.get();
			if (dst && dst->AsActorValueOwner()) {
				auto inventoryWeight =
					static_cast<std::ptrdiff_t>(dst->GetWeightInContainer());
				auto carryWeight =
					static_cast<std::ptrdiff_t>(dst->AsActorValueOwner()->GetActorValue(RE::ActorValue::kCarryWeight));
				auto text = std::to_string(inventoryWeight);
				text += " / ";
				text += std::to_string(carryWeight);
				_weight.HTMLText(text);
				_weight.Visible(true);
			}
		}

		[[nodiscard]] bool WouldBeStealing() const
		{
			auto dst = _dst.get();
			auto src = _src.get();
			return dst && src && dst->WouldBeStealing(src.get());
		}

		static constexpr std::string_view FILE_NAME{ "LootMenuIE" };
		static constexpr std::string_view MENU_NAME{ "LootMenu" };
		static constexpr std::int8_t SORT_PRIORITY{ 3 };

		RE::GPtr<RE::GFxMovieView> _view;
		RE::ActorHandle _dst{ RE::PlayerCharacter::GetSingleton() };
		RE::ObjectRefHandle _src;

		std::optional<ViewHandler> _viewHandler;
		ContainerChangedHandler _containerChangedHandler;
		OpenCloseHandler _openCloseHandler{ _dst };

		CLIK::MovieClip _rootObj;
		CLIK::TextField _title;
		CLIK::TextField _weight;

		CLIK::GFx::Controls::ScrollingList _itemList;
		RE::GFxValue _itemListProvider;
		std::vector<std::unique_ptr<QuickLoot::Items::OldItem>> _itemListImpl;

		CLIK::GFx::Controls::ButtonBar _infoBar;
		RE::GFxValue _infoBarProvider;

		CLIK::GFx::Controls::ButtonBar _buttonBar;
		RE::GFxValue _buttonBarProvider;
	};
}
