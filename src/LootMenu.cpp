#include "LootMenu.h"

#include "Behaviors/ContainerAnimator.h"
#include "CLIK/Array.h"
#include "CLIK/GFx/Controls/ButtonBar.h"
#include "CLIK/GFx/Controls/ScrollingList.h"
#include "CLIK/TextField.h"
#include "Config/SystemSettings.h"
#include "Config/UserSettings.h"
#include "Input/ButtonArt.h"
#include "Input/InputManager.h"
#include "Integrations/APIServer.h"
#include "Items/ItemStack.h"
#include "LootMenuManager.h"

#include <numbers>

#undef PlaySound

namespace QuickLoot
{
	void LootMenu::Register()
	{
		RE::UI::GetSingleton()->Register(MENU_NAME, CreateInstance);
		logger::info("Registered {}"sv, MENU_NAME);
	}

	RE::IMenu* LootMenu::CreateInstance()
	{
		logger::debug("Creating new loot menu instance");
		return new LootMenu();
	}

	int LootMenu::GetSwfVersion()
	{
		IMenu dummy{};
		RE::GPtr<RE::GFxMovieView> movieView{};
		RE::BSScaleformManager::GetSingleton()->LoadMovie(&dummy, movieView, FILE_NAME.data());

		if (!movieView)
			return -2;

		RE::GFxValue lootMenuObj;
		movieView->GetVariable(&lootMenuObj, "_root.lootMenu");

		if (!lootMenuObj.IsObject())
			return -1;

		RE::GFxValue version;
		if (!lootMenuObj.Invoke("getVersion", &version) || !version.IsNumber())
			return 0;

		return static_cast<int>(version.GetSInt());
	}

#pragma region Initialization

	LootMenu::LootMenu() :
		UniversalMenu(false, false, false)
	{
		PROFILE_SCOPE_NAMED("LootMenu Constructor");

		depthPriority = 3;
		menuName = MENU_NAME;
		menuFlags.set(Flag::kAllowSaving, Flag::kHasButtonBar);

		if (_cachedView) {
			logger::debug("Using cached swf");
			uiMovie = _cachedView;
		} else {
			logger::debug("Loading swf");
			PROFILE_SCOPE_NAMED("SWF Loading");

			RE::BSScaleformManager::GetSingleton()->LoadMovie(this, uiMovie, FILE_NAME.data());

			if (Config::SystemSettings::EnableMenuCaching()) {
				_cachedView = uiMovie;
			}
		}

		if (!uiMovie) {
			logger::error("Failed to load {}.swf", FILE_NAME);
			LootMenuManager::RequestHide();
			return;
		}

		// Disable input, since we'll handle it in C++.
		uiMovie->SetMouseCursorCount(0);
		uiMovie->SetVariable("_global.gfxExtensions", true);
		InjectUtilsClass();

		LoadSwfObject(_lootMenu, "_root.lootMenu"sv);
		LoadSwfObject(_title, "_root.lootMenu.title"sv);
		LoadSwfObject(_weight, "_root.lootMenu.weight"sv);
		LoadSwfObject(_itemList, "_root.lootMenu.itemList"sv);
		LoadSwfObject(_infoBar, "_root.lootMenu.infoBar"sv);
		LoadSwfObject(_buttonBar, "_root.lootMenu.buttonBar"sv);

		RE::GFxValue version;
		if (_lootMenu.GetInstance().Invoke("getVersion", &version) && version.IsNumber()) {
			_swfFeatureLevel = static_cast<int>(version.GetNumber());
		}

		if (_swfFeatureLevel < 4) {
			// swf reinitialization is only supported at feature level 4 or above
			_cachedView.reset();
		}

		_lootMenu.Visible(false);

		_title.AutoSize(CLIK::Object{ "left" });
		_weight.AutoSize(CLIK::Object{ "left" });

		uiMovie->CreateArray(std::addressof(_itemListProvider));
		_itemList.DataProvider(CLIK::Array{ _itemListProvider });

		uiMovie->CreateArray(std::addressof(_infoBarProvider));
		_infoBar.DataProvider(CLIK::Array{ _infoBarProvider });

		uiMovie->CreateArray(std::addressof(_buttonBarProvider));
		_buttonBar.DataProvider(CLIK::Array{ _buttonBarProvider });

		{
			PROFILE_SCOPE_NAMED("Initial Task Processing");
			// This is where Show is called.
			LootMenuManager::ProcessPendingTasks(*this);
		}

		if (REL::Module::IsVR()) {
			UniversalMenu::SetupMenuNode();
		}
	}

	void LootMenu::InjectUtilsClass()
	{
		class ProxyFunctionHandler : public RE::GFxFunctionHandler
		{
			std::function<void(Params& params)> _handler;

			void Call(Params& params) override
			{
				_handler(params);
			}

		public:
			ProxyFunctionHandler(const std::function<void(Params& params)>& handler) :
				_handler(handler) {}
		};

		static ProxyFunctionHandler logger([this](auto& params) {
			if (params.argCount == 0 || !params.args[0].IsString()) {
				return;
			}

			logger::info("! {}", params.args[0].GetString());
		});

		RE::GFxValue quickLootUtilsClass;
		RE::GFxValue quickLootLogFunction;

		uiMovie->CreateObject(&quickLootUtilsClass);
		uiMovie->CreateFunction(&quickLootLogFunction, &logger, nullptr);

		uiMovie->SetVariable("_global.QuickLoot.Utils", quickLootUtilsClass);
		uiMovie->SetVariable("_global.QuickLoot.Utils.log", quickLootLogFunction);
	}

	void LootMenu::LoadSwfObject(CLIK::Object& target, std::string_view path) const
	{
		auto& instance = target.GetInstance();
		bool success = uiMovie->GetVariable(std::addressof(instance), path.data());

		if (!success || !instance.IsObject()) {
			logger::error("Failed to find variable {} in {}.swf", path, FILE_NAME);
		}
	}

	RE::GFxValue LootMenu::BuildSettingsObject() const
	{
		PROFILE_SCOPE;

		RE::GFxValue settings{};

		uiMovie->CreateObject(&settings);

		settings.SetMember("minLines", Settings::GetWindowMinLines());
		settings.SetMember("maxLines", Settings::GetWindowMaxLines());

		settings.SetMember("offsetX", Settings::GetWindowX());
		settings.SetMember("offsetY", Settings::GetWindowY());
		settings.SetMember("scale", Settings::GetWindowScale());

		settings.SetMember("alphaNormal", Settings::GetWindowOpacityNormal());
		settings.SetMember("alphaEmpty", Settings::GetWindowOpacityEmpty());

		double anchorFractionX = 0;
		double anchorFractionY = 0;
		ResolveAnchorPoint(Settings::GetWindowAnchor(), anchorFractionX, anchorFractionY);

		settings.SetMember("anchorFractionX", anchorFractionX);
		settings.SetMember("anchorFractionY", anchorFractionY);

		RE::GFxValue infoColumns{};
		uiMovie->CreateArray(&infoColumns);

		// Skyrim's Papyrus string caching can cause the column names to arrive with
		// incorrect capitalization, so we need to normalize.
		constexpr std::array normalizedColumnNames = {
			"value",
			"weight",
			"valuePerWeight"
		};

		for (const auto& column : Settings::GetInfoColumns()) {
			for (const auto& normalized : normalizedColumnNames) {
				if (_stricmp(column.c_str(), normalized) == 0) {
					infoColumns.PushBack(normalized);
					break;
				}
			}
		}

		settings.SetMember("infoColumns", infoColumns);

		settings.SetMember("showItemIcons", Settings::ShowIconItem());

		return settings;
	}

	void LootMenu::ResolveAnchorPoint(Config::AnchorPoint anchor, double& fractionX, double& fractionY)
	{
		using enum Config::AnchorPoint;
		switch (anchor) {
		case kTopLeft:
		case kCenterLeft:
		case kBottomLeft:
			fractionX = 0.0;
			break;

		case kTopCenter:
		case kCenter:
		case kBottomCenter:
			fractionX = 0.5;
			break;

		case kTopRight:
		case kCenterRight:
		case kBottomRight:
			fractionX = 1.0;
			break;
		}

		switch (anchor) {
		case kTopLeft:
		case kTopCenter:
		case kTopRight:
			fractionY = 0.0;
			break;

		case kCenterLeft:
		case kCenter:
		case kCenterRight:
			fractionY = 0.5;
			break;

		case kBottomLeft:
		case kBottomCenter:
		case kBottomRight:
			fractionY = 1.0;
			break;
		}
	}

	void LootMenu::Show(const RE::ObjectRefHandle& container, int selectedIndex)
	{
		if (!uiMovie) {
			return;
		}

		if (_container) {
			LootMenuManager::SaveLastSelectedIndex(_container, _selectedIndex);
			API::APIServer::DispatchCloseLootMenuEvent(_container.get().get());
		}

		_container = container;

		if (_container) {
			// swf reinitialization is only supported at feature level 4 or above
			if (!_wasInitialized || _swfFeatureLevel >= 4) {
				const RE::GFxValue settings = BuildSettingsObject();
				_lootMenu.GetInstance().Invoke("init", nullptr, &settings, 1);
				_wasInitialized = true;
			}

			API::APIServer::DispatchOpenLootMenuEvent(_container.get().get());
		}

		_lootMenu.Visible(container.get() != nullptr);

		Refresh(RefreshFlags::kAll);
		SetSelectedIndex(selectedIndex, false);
	}

	void LootMenu::Hide()
	{
		Show({}, -1);
	}

#pragma endregion

#pragma region Actions

	void LootMenu::OnInputAction(Input::QuickLootAction action)
	{
		if (!uiMovie) {
			return;
		}

		switch (action) {
		case Input::QuickLootAction::kTake:
			TakeStack();
			break;

		case Input::QuickLootAction::kTakeAll:
			TakeAll();
			break;

		case Input::QuickLootAction::kTransfer:
			Transfer();
			break;

		case Input::QuickLootAction::kScrollUp:
			ScrollUp();
			break;

		case Input::QuickLootAction::kScrollDown:
			ScrollDown();
			break;

		case Input::QuickLootAction::kPrevPage:
			ScrollPrevPage();
			break;

		case Input::QuickLootAction::kNextPage:
			ScrollNextPage();
			break;

		default:
			break;
		}
	}

	void LootMenu::OnSelectedIndexChanged(int newIndex)
	{
		QueueRefresh(RefreshFlags::kInfoBar);

		if (newIndex < 0 || newIndex >= std::ssize(_inventory)) {
			return;
		}

		_inventory[static_cast<size_t>(newIndex)].OnSelected(RE::PlayerCharacter::GetSingleton());
	}

	void LootMenu::SetSelectedIndex(int newIndex, bool playSound)
	{
		if (newIndex < 0) {
			newIndex = 0;
		}

		// This sets the index to -1 if the container is empty.
		if (newIndex >= _inventory.size()) {
			newIndex = static_cast<int>(_inventory.size() - 1);
		}

		if (newIndex == _selectedIndex) {
			return;
		}

		if (playSound && Config::UserSettings::PlayScrollSound()) {
			RE::PlaySound("UIMenuFocus");
		}

		_selectedIndex = newIndex;
		_itemList.SelectedIndex(newIndex);
		OnSelectedIndexChanged(newIndex);
	}

	void LootMenu::ScrollUp()
	{
		SetSelectedIndex(std::max(_selectedIndex - 1, 0), true);
	}

	void LootMenu::ScrollDown()
	{
		SetSelectedIndex(std::min(_selectedIndex + 1, static_cast<int>(_inventory.size()) - 1), true);
	}

	void LootMenu::ScrollPrevPage()
	{
		// TODO implement
	}

	void LootMenu::ScrollNextPage()
	{
		// TODO implement
	}

	void LootMenu::DispelEffectsWithArchetype(RE::MagicTarget* target, RE::MagicTarget::Archetype type, bool force)
	{
		if (!target || !target->GetActiveEffectList()) {
			return;
		}

		for (auto* effect : *target->GetActiveEffectList()) {
			if (effect && effect->GetBaseObject() && effect->GetBaseObject()->HasArchetype(type)) {
				effect->Dispel(force);
			}
		}
	}

	void LootMenu::OnTakeAction()
	{
		const auto player = RE::PlayerCharacter::GetSingleton();

		Behaviors::ContainerAnimator::OpenContainer(_container);

		// Taken from WaterFox' fork of QuickLootEE
		// See: https://github.com/Eloquence4/QuickLootEE/blob/c93e56dcb7f0372a5ad7df4b22e118e37deeb286/src/Scaleform/LootMenu.h#L136-L162
		if (Settings::DispelInvisibility()) {
			DispelEffectsWithArchetype(player->AsMagicTarget(), RE::EffectArchetypes::ArchetypeID::kInvisibility, false);
		}

		QueueRefresh(RefreshFlags::kInventory);
	}

	void LootMenu::TakeStack()
	{
		const auto player = RE::PlayerCharacter::GetSingleton();

		if (_selectedIndex < 0 || _selectedIndex >= _inventory.size()) {
			logger::warn("Failed to take stack at index {} ({} entries)", _selectedIndex, _inventory.size());
			return;
		}

		_inventory[_selectedIndex].TakeStack(player);

		OnTakeAction();
	}

	void LootMenu::TakeAll()
	{
		const auto player = RE::PlayerCharacter::GetSingleton();

		for (size_t i = 0; i < _inventory.size(); ++i) {
			_inventory[i].TakeStack(player);
		}

		OnTakeAction();
	}

	void LootMenu::Transfer()
	{
		RE::PlayerCharacter::GetSingleton()->ActivatePickRef();
	}

#pragma endregion

#pragma region Refresh

	void LootMenu::QueueRefresh(RefreshFlags flags)
	{
		_refreshFlags |= flags;
	}

	void LootMenu::Refresh(RefreshFlags flags)
	{
		QueueRefresh(flags);

		if (!uiMovie || !_refreshFlags) {
			return;
		}

		PROFILE_SCOPE

		if (_refreshFlags & RefreshFlags::kInventory) {
			// Inventory refresh invalidates other components, so it needs to run first.
			RefreshInventory();
		}

		if (_refreshFlags & RefreshFlags::kButtonBar) {
			RefreshButtonBar();
		}

		if (_refreshFlags & RefreshFlags::kInfoBar) {
			RefreshInfoBar();
		}

		if (_refreshFlags & RefreshFlags::kWeight) {
			RefreshWeight();
		}

		if (_refreshFlags & RefreshFlags::kTitle) {
			RefreshTitle();
		}

		_refreshFlags = RefreshFlags::kNone;
	}

	using SortFunction = std::function<int(const Items::QuickLootItemStack& a, const Items::QuickLootItemStack& b)>;

	template <Items::ItemType type>
	bool IsType(const Items::QuickLootItemStack& stack)
	{
		return stack.GetData().type == type;
	}

	template <Items::MiscType type>
	bool IsMisc(const Items::QuickLootItemStack& stack)
	{
		const auto& data = stack.GetData();
		return data.type == Items::ItemType::kMisc && data.misc.subType.valid && data.misc.subType == type;
	}

	template <typename T>
	SortFunction OrderAsc(T(*func)(const Items::QuickLootItemStack&))
	{
		return [=](const Items::QuickLootItemStack& a, const Items::QuickLootItemStack& b) {
			const auto diff = func(a) - func(b);
			return diff < 0 ? -1 : diff > 0 ? 1 : 0;
		};
	}

	template <typename T>
	SortFunction OrderDesc(T (*func)(const Items::QuickLootItemStack&))
	{
		return [=](const Items::QuickLootItemStack& a, const Items::QuickLootItemStack& b) {
			const auto diff = func(b) - func(a);
			return diff < 0 ? -1 : diff > 0 ? 1 : 0;
		};
	}

	void LootMenu::SortInventory()
	{
		PROFILE_SCOPE;

		std::vector<SortFunction> selectedRules{};

		static std::map<std::string, SortFunction> ruleTable = {
			{ "$qlie_SortRule_Gold", OrderDesc(IsMisc<Items::MiscType::kGold>) },
			{ "$qlie_SortRule_Gems", OrderDesc(IsMisc<Items::MiscType::kGem>) },
			{ "$qlie_SortRule_SoulGems", OrderDesc(IsType<Items::ItemType::kSoulGem>) },
			{ "$qlie_SortRule_Lockpicks", OrderDesc(IsMisc<Items::MiscType::kLockpick>) },
			{ "$qlie_SortRule_OresIngots", OrderDesc(IsMisc<Items::MiscType::kIngot>) },
			{ "$qlie_SortRule_Potions", OrderDesc<bool>([](const Items::QuickLootItemStack& stack) {
				const auto& data = stack.GetData();
				return data.formType == RE::FormType::AlchemyItem
					&& data.potion.subType != Items::PotionType::kFood
					&& data.potion.subType != Items::PotionType::kDrink;
			}) },
			{ "$qlie_SortRule_FoodDrinks", OrderDesc<bool>([](const Items::QuickLootItemStack& stack) {
				const auto& data = stack.GetData();
				return data.formType == RE::FormType::AlchemyItem
					&& (data.potion.subType == Items::PotionType::kFood
					|| data.potion.subType == Items::PotionType::kDrink);
			}) },
			{ "$qlie_SortRule_Books", OrderDesc<bool>([](const Items::QuickLootItemStack& stack) {
				const auto& data = stack.GetData();
				return data.formType == RE::FormType::Book
					&& (!data.book.subType.valid
					|| data.book.subType != Items::BookSubType::kNote
					&& data.book.subType != Items::BookSubType::kRecipe);
			}) },
			{ "$qlie_SortRule_Notes", OrderDesc<bool>([](const Items::QuickLootItemStack& stack) {
				const auto& data = stack.GetData();
				return data.formType == RE::FormType::Book
					&& data.book.subType.valid
					&& (data.book.subType == Items::BookSubType::kNote
					|| data.book.subType == Items::BookSubType::kRecipe);
			}) },
			{ "$qlie_SortRule_Scrolls", OrderDesc<bool>([](const Items::QuickLootItemStack& stack) {
				return stack.GetData().formType == RE::FormType::Scroll;
			}) },
			{ "$qlie_SortRule_Weapons", OrderDesc<bool>([](const Items::QuickLootItemStack& stack) {
				const auto& data = stack.GetData();
				return data.formType == RE::FormType::Weapon;
			}) },
			{ "$qlie_SortRule_ArrowsBolts", OrderDesc<bool>([](const Items::QuickLootItemStack& stack) {
				return stack.GetData().formType == RE::FormType::Ammo;
			}) },
			{ "$qlie_SortRule_Armors", OrderDesc<bool>([](const Items::QuickLootItemStack& stack) {
				const auto& data = stack.GetData();
				return data.formType == RE::FormType::Armor
					&& data.armor.weightClass != Items::ArmorWeightClass::kClothing
					&& data.armor.weightClass != Items::ArmorWeightClass::kJewelry;
			}) },
			{ "$qlie_SortRule_Clothes", OrderDesc<bool>([](const Items::QuickLootItemStack& stack) {
				const auto& data = stack.GetData();
				return data.formType == RE::FormType::Armor
					&& data.armor.weightClass == Items::ArmorWeightClass::kClothing;
			}) },
			{ "$qlie_SortRule_Jewelry", OrderDesc<bool>([](const Items::QuickLootItemStack& stack) {
				const auto& data = stack.GetData();
				return data.formType == RE::FormType::Armor
					&& data.armor.weightClass == Items::ArmorWeightClass::kJewelry;
			}) },
			{ "$qlie_SortRule_Weightless", OrderDesc<bool>([](const Items::QuickLootItemStack& stack) { return stack.GetData().weight.value <= 0; }) },
			{ "$qlie_SortRule_ByWeight", OrderAsc<float>([](const Items::QuickLootItemStack& stack) { return stack.GetData().weight.value; }) },
			{ "$qlie_SortRule_ByValue", OrderDesc<int>([](const Items::QuickLootItemStack& stack) { return stack.GetData().value.value; }) },
			{ "$qlie_SortRule_ByV/W", OrderDesc<float>([](const Items::QuickLootItemStack& stack) {
				const auto data = stack.GetData();
				if (data.weight <= 0)
					return std::numeric_limits<float>::infinity();
				if (data.value <= 0)
					return 0.0f;
				return data.infoValueWeight.value;
			}) },
			{ "$qlie_SortRule_ByName", [](const Items::QuickLootItemStack& a, const Items::QuickLootItemStack& b) {
				 const auto& nameA = a.GetQuickLootData().displayName.value.c_str();
				 const auto& nameB = b.GetQuickLootData().displayName.value.c_str();
				 return strcmp(nameA, nameB) < 0;
			 } },
			{ "$qlie_SortRule_ArtifactNeeded", OrderDesc<bool>([](const Items::QuickLootItemStack& stack) { return stack.GetQuickLootData().dbmNew.value; }) },
			{ "$qlie_SortRule_CompletionistNeeded", OrderDesc<bool>([](const Items::QuickLootItemStack& stack) { return stack.GetQuickLootData().compNew.value; }) },
		};

		for (auto& ruleName : Config::UserSettings::GetActiveSortRules()) {
			const auto it = ruleTable.find(ruleName);
			if (it != ruleTable.end()) {
				selectedRules.push_back(it->second);
			}
		}

		std::ranges::stable_sort(_inventory, [&](const Items::QuickLootItemStack& a, const Items::QuickLootItemStack& b) {
			for (auto ruleFunction : selectedRules) {
				const auto cmp = ruleFunction(a, b);
				if (cmp < 0)
					return true;
				if (cmp > 0)
					return false;
			}

			const auto& nameA = a.GetQuickLootData().displayName.value.c_str();
			const auto& nameB = b.GetQuickLootData().displayName.value.c_str();
			return strcmp(nameA, nameB) < 0;
		});
	}

	void LootMenu::RefreshInventory()
	{
		PROFILE_SCOPE;

		_itemListProvider.ClearElements();

		const auto container = _container.get();
		if (!container) {
			_itemList.Invalidate();
			_itemList.SelectedIndex(-1.0);
			return;
		}

		_inventory = Items::QuickLootItemStack::LoadContainerInventory(container.get(), CanDisplay);
		if (_inventory.empty() && !Settings::ShowWhenEmpty()) {
			LootMenuManager::RequestHide();
			return;
		}

		SortInventory();

		std::vector<API::ItemStack> apiInventory;
		apiInventory.reserve(_inventory.size());
		for (auto& item : _inventory) {
			_itemListProvider.PushBack(item.BuildDataObject(uiMovie.get()));
			apiInventory.emplace_back(item.GetEntry(), item.GetDropRef().get().get());
		}

		_itemList.InvalidateData();
		_lootMenu.GetInstance().Invoke("refresh");

		API::APIServer::DispatchInvalidateLootMenuEvent(_container.get().get(), apiInventory);
		SetSelectedIndex(_selectedIndex, false);

		QueueRefresh(RefreshFlags::kWeight);
		QueueRefresh(RefreshFlags::kInfoBar);
	}

	void LootMenu::RefreshButtonBar()
	{
		if (!uiMovie) {
			return;
		}

		PROFILE_SCOPE;

		const auto keybindings = Input::InputManager::GetButtonBarKeybindings();
		const bool stealing = WouldBeStealing();

		_buttonBarProvider.ClearElements();

		for (const auto& keybinding : keybindings) {
			const auto label = GetActionDisplayName(keybinding.action, stealing);
			const auto index = keybinding.buttonArtOverride != Input::ButtonArtIndex::kNone ? keybinding.buttonArtOverride : Input::ButtonArt::GetFrameIndexForDeviceKey(keybinding.deviceType, keybinding.inputKey);

			RE::GFxValue obj;
			uiMovie->CreateObject(&obj);

			obj.SetMember("label", label);
			obj.SetMember("index", index);
			obj.SetMember("stolen", stealing);

			_buttonBarProvider.PushBack(obj);
		}

		_buttonBar.InvalidateData();
	}

	void LootMenu::RefreshInfoBar()
	{
		PROFILE_SCOPE;

		/* TODO
		_infoBarProvider.ClearElements();
		const auto idx = static_cast<std::ptrdiff_t>(_itemList.SelectedIndex());
		if (0 <= idx && idx < std::ssize(_inventory)) {
			typedef std::function<std::string(const QuickLoot::Items::OldItem&)> functor;
			const std::array functors{
				functor{ [](const QuickLoot::Items::OldItem& a_val) { return fmt::format(FMT_STRING("{:.1f}"), a_val.Weight()); } },
				functor{ [](const QuickLoot::Items::OldItem& a_val) { return fmt::format(FMT_STRING("{}"), a_val.Value()); } },
			};

			const auto& item = _inventory[static_cast<size_t>(idx)];
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
		*/
	}

	void LootMenu::RefreshWeight()
	{
		PROFILE_SCOPE;

		const auto player = RE::PlayerCharacter::GetSingleton();
		const auto currentWeight = static_cast<int64_t>(player->GetWeightInContainer());
		const auto carryWeightLimit = static_cast<int64_t>(player->GetTotalCarryWeight());
		const auto text = fmt::format("{} / {}", currentWeight, carryWeightLimit);
		_weight.HTMLText(text);
		_weight.Visible(true);
	}

	void LootMenu::RefreshTitle()
	{
		PROFILE_SCOPE;

		if (const auto container = _container.get()) {
			const auto name = container->GetDisplayFullName();
			_title.HTMLText(name ? name : "");
			_title.Visible(true);
		}
	}

	const char* LootMenu::GetActionDisplayName(Input::QuickLootAction action, bool stealing)
	{
		struct ActionDefinition
		{
			const char* labelKey;
			const char* labelFallback;

			const char* stealingLabelKey;
			const char* stealingLabelFallback;
		};

		constexpr std::array actionDefinitions{
			ActionDefinition{ "sTake", "Take", "sSteal", "Steal" },
			ActionDefinition{ "sTakeAll", "Take All", "sTakeAll", "Take All" },
			ActionDefinition{ "sSearch", "Search", "sStealFrom", "Steal From" },
		};

		int actionIndex;
		switch (action) {
		case Input::QuickLootAction::kTake:
			actionIndex = 0;
			break;

		case Input::QuickLootAction::kTakeAll:
			actionIndex = 1;
			break;

		case Input::QuickLootAction::kTransfer:
			actionIndex = 2;
			break;

		default:
			return "<invalid>";
		}

		const auto& button = actionDefinitions[actionIndex];

		const auto labelKey = stealing ? button.stealingLabelKey : button.labelKey;
		const auto labelFallback = stealing ? button.stealingLabelFallback : button.labelFallback;

		const auto setting = RE::GameSettingCollection::GetSingleton()->GetSetting(labelKey);
		const auto label = setting ? setting->GetString() : labelFallback;

		return label;
	}

	bool LootMenu::CanDisplay(const RE::TESBoundObject& object)
	{
		switch (object.GetFormType()) {
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
				auto& light = static_cast<const RE::TESObjectLIGH&>(object);
				if (!light.CanBeCarried()) {
					return false;
				}
			}
			break;
		default:
			return false;
		}

		if (!object.GetPlayable()) {
			return false;
		}

		auto name = object.GetName();
		if (!name || name[0] == '\0') {
			return false;
		}

		return true;
	}

	bool LootMenu::WouldBeStealing() const
	{
		const auto container = _container.get().get();
		return container && RE::PlayerCharacter::GetSingleton()->WouldBeStealing(container);
	}

#pragma endregion

#pragma region IMenu implementation

	RE::UI_MESSAGE_RESULTS LootMenu::ProcessMessage(RE::UIMessage& message)
	{
		if (message.type == RE::UI_MESSAGE_TYPE::kHide) {
			Hide();

			return RE::UI_MESSAGE_RESULTS::kHandled;
		}

		return UniversalMenu::ProcessMessage(message);
	}

	void LootMenu::AdvanceMovie(float interval, std::uint32_t currentTime)
	{
		// First run all queued tasks, which may set refresh flags
		LootMenuManager::ProcessPendingTasks(*this);

		// Refresh any components that require updates
		Refresh();

		// Redraw visuals
		UniversalMenu::AdvanceMovie(interval, currentTime);
	}

	void LootMenu::RefreshPlatform()
	{
		QueueRefresh(RefreshFlags::kButtonBar);
	}

#pragma endregion

#pragma region VR

	RE::NiPointer<RE::NiNode> LootMenu::GetAttachingNode()
	{
		auto& vrData = RE::PlayerCharacter::GetSingleton()->GetVRPlayerRuntimeData();
		return vrData.isRightHandMainHand ? vrData.RightWandNode : vrData.LeftWandNode;
	}

	void LootMenu::SetTransform()
	{
		const float dx = Settings::VrOffsetX();
		const float dy = Settings::VrOffsetY();
		const float dz = Settings::VrOffsetZ();

		const float rx = static_cast<float>(Settings::VrAngleX() * std::numbers::pi / 180);
		const float ry = static_cast<float>(Settings::VrAngleY() * std::numbers::pi / 180);
		const float rz = static_cast<float>(Settings::VrAngleZ() * std::numbers::pi / 180);

		const float scale = Settings::VrScale();

		menuNode->local.translate = RE::NiPoint3(dx, dy, dz);
		menuNode->local.rotate.EulerAnglesToAxesZXY(rx, ry, rz);
		menuNode->local.scale = scale;

		GetAttachingNode()->AttachChild(menuNode.get(), true);
		RE::NiUpdateData data{};
		menuNode->Update(data);
	}

	RE::NiNode* LootMenu::GetMenuParentNode()
	{
		return RE::PlayerCharacter::GetSingleton()->GetVRNodeData()->UprightHmdNode.get();
	}

	RE::BSEventNotifyControl LootMenu::ProcessEvent(const RE::HudModeChangeEvent*, RE::BSTEventSource<RE::HudModeChangeEvent>*)
	{
		return RE::BSEventNotifyControl::kContinue;
	}

#pragma endregion
}
