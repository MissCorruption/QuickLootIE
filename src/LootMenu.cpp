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
#include "Integrations/NPCsNamesDistributor.h"
#include "Items/Inventory.h"
#include "Items/ItemStack.h"
#include "LootMenuManager.h"
#include "MenuVisibilityManager.h"
#include "Util/ScaleformUtil.h"

#include <numbers>

#undef PlaySound

using namespace QuickLoot::Config;

namespace QuickLoot
{
	void LootMenu::Register()
	{
		RE::UI::GetSingleton()->Register(MENU_NAME, CreateInstance);
		logger::info("Installed {} as {}"sv, typeid(LootMenu).name(), MENU_NAME);
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

		if (SystemSettings::EnableMenuCaching() && _cachedView) {
			logger::debug("Using cached swf");
			uiMovie = _cachedView;
		} else {
			logger::debug("Loading swf");
			PROFILE_SCOPE_NAMED("SWF Loading");

			RE::BSScaleformManager::GetSingleton()->LoadMovie(this, uiMovie, FILE_NAME.data());

			_cachedView = SystemSettings::EnableMenuCaching() ? uiMovie : nullptr;
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

		settings.SetMember("minLines", UserSettings::GetWindowMinLines());
		settings.SetMember("maxLines", UserSettings::GetWindowMaxLines());

		settings.SetMember("offsetX", UserSettings::GetWindowX());
		settings.SetMember("offsetY", UserSettings::GetWindowY());
		settings.SetMember("scale", UserSettings::GetWindowScale());

		settings.SetMember("alphaNormal", UserSettings::GetWindowOpacityNormal());
		settings.SetMember("alphaEmpty", UserSettings::GetWindowOpacityEmpty());

		double anchorFractionX = 0;
		double anchorFractionY = 0;
		ResolveAnchorPoint(UserSettings::GetWindowAnchor(), anchorFractionX, anchorFractionY);

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

		for (const auto& column : UserSettings::GetInfoColumns()) {
			for (const auto& normalized : normalizedColumnNames) {
				if (_stricmp(column.c_str(), normalized) == 0) {
					infoColumns.PushBack(normalized);
					break;
				}
			}
		}

		settings.SetMember("infoColumns", infoColumns);

		settings.SetMember("showItemIcons", UserSettings::ShowIconItem());

		settings.SetMember("systemSettings", Util::ScaleformUtil::BuildGFxValueFromJson(uiMovie.get(), SystemSettings::GetSwfConfig()));

		return settings;
	}

	void LootMenu::ResolveAnchorPoint(AnchorPoint anchor, double& fractionX, double& fractionY)
	{
		using enum AnchorPoint;
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
			API::APIServer::DispatchCloseLootMenuEvent(_container);
		}

		_container = container;

		if (_container) {
			// swf reinitialization is only supported at feature level 4 or above
			if (!_wasInitialized || _swfFeatureLevel >= 4) {
				const RE::GFxValue settings = BuildSettingsObject();
				_lootMenu.GetInstance().Invoke("init", nullptr, &settings, 1);
				_wasInitialized = true;
			}

			API::APIServer::DispatchOpenLootMenuEvent(_container);
		}

		_lootMenu.Visible(container.get() != nullptr);

		_refreshFlags = RefreshFlags::kNone;
		RefreshInventory(false);
		SetSelectedIndex(selectedIndex, false);
		Refresh(RefreshFlags::kAllButInventory);
	}

	void LootMenu::Hide()
	{
		Show({}, -1);
	}

#pragma endregion

#pragma region Actions

	void LootMenu::OnInputAction(Input::QuickLootAction action)
	{
		if (!uiMovie || RE::UI::GetSingleton()->IsMenuOpen(RE::Console::MENU_NAME)) {
			return;
		}

		switch (action) {
		case Input::QuickLootAction::kUse:
			UseItem();
			break;

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

	void LootMenu::SetSelectedIndex(int newIndex, bool playSound)
	{
		newIndex = std::max(newIndex, 0);

		// This sets the index to -1 if the container is empty.
		if (newIndex >= _inventory.size()) {
			newIndex = static_cast<int>(_inventory.size() - 1);
		}

		if (newIndex != _selectedIndex) {
			if (playSound && UserSettings::PlayScrollSound()) {
				RE::PlaySound("UIMenuFocus");
			}
		}

		_selectedIndex = newIndex;
		_itemList.SelectedIndex(newIndex);

		QueueRefresh(RefreshFlags::kInfoBar);
		QueueRefresh(RefreshFlags::kButtonBar);

		if (newIndex >= 0 && newIndex < std::ssize(_inventory)) {
			const auto& stack = _inventory[static_cast<size_t>(newIndex)];
			API::APIServer::DispatchSelectItemEvent(RE::PlayerCharacter::GetSingleton(), _container, stack->GetEntry(), stack->GetDropRef());
		}
	}

	void LootMenu::ScrollUp()
	{
		if (_selectedIndex > 0) {
			SetSelectedIndex(_selectedIndex - 1, true);
		}
	}

	void LootMenu::ScrollDown()
	{
		if (_selectedIndex < _inventory.size() - 1) {
			SetSelectedIndex(_selectedIndex + 1, true);
		}
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
		if (UserSettings::DispelInvisibility()) {
			DispelEffectsWithArchetype(player->AsMagicTarget(), RE::EffectArchetypes::ArchetypeID::kInvisibility, false);
		}

		QueueRefresh(RefreshFlags::kInventory);
	}

	void LootMenu::TakeStack()
	{
		PROFILE_SCOPE

		const auto player = RE::PlayerCharacter::GetSingleton();

		if (_selectedIndex < 0 || _selectedIndex >= _inventory.size()) {
			logger::warn("Failed to take stack at index {} ({} entries)", _selectedIndex, _inventory.size());
			return;
		}

		auto& stack = _inventory[_selectedIndex];

		if (API::APIServer::DispatchTakingItemEvent(player, _container, stack->GetEntry(), stack->GetDropRef()) != API::HandleResult::kStop) {
			stack->TakeStack(player);

			API::APIServer::DispatchTakeItemEvent(player, _container, stack->GetEntry(), stack->GetDropRef());
		}

		OnTakeAction();
	}

	void LootMenu::TakeAll()
	{
		PROFILE_SCOPE

		const auto player = RE::PlayerCharacter::GetSingleton();

		for (size_t i = 0; i < _inventory.size(); ++i) {
			auto& stack = _inventory[i];

			if (API::APIServer::DispatchTakingItemEvent(player, _container, stack->GetEntry(), stack->GetDropRef()) != API::HandleResult::kStop) {
				stack->TakeStack(player);

				API::APIServer::DispatchTakeItemEvent(player, _container, stack->GetEntry(), stack->GetDropRef());
			}
		}

		OnTakeAction();
	}

	void LootMenu::Transfer()
	{
		RE::PlayerCharacter::GetSingleton()->ActivatePickRef();
	}

	void LootMenu::UseItem()
	{
		PROFILE_SCOPE

		const auto player = RE::PlayerCharacter::GetSingleton();

		if (_selectedIndex < 0 || _selectedIndex >= _inventory.size()) {
			logger::warn("Failed to use stack at index {} ({} entries)", _selectedIndex, _inventory.size());
			return;
		}

		auto& stack = _inventory[_selectedIndex];

		if (API::APIServer::DispatchTakingItemEvent(player, _container, stack->GetEntry(), stack->GetDropRef()) != API::HandleResult::kStop) {
			stack->Use(player);

			API::APIServer::DispatchTakeItemEvent(player, _container, stack->GetEntry(), stack->GetDropRef());
		}

		OnTakeAction();
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

	void LootMenu::LoadInventory()
	{
		PROFILE_SCOPE

		auto inventory = Items::Inventory::LoadContainerInventory(_container.get().get(), CanDisplay);

		API::APIServer::DispatchModifyInventoryEvent(_container, inventory);

		if (inventory.empty() && !UserSettings::ShowWhenEmpty() && !MenuVisibilityManager::IsForcedContainer(_container)) {
			LootMenuManager::RequestHide();
			return;
		}

		API::APIServer::DispatchInvalidateLootMenuEvent(_container, inventory);

		{
			PROFILE_SCOPE_NAMED("Item data")
			_inventory.clear();
			_inventory.reserve(inventory.size());

			for (auto stack : inventory) {
				auto s = std::make_unique<Items::QuickLootItemStack>(stack.entry, _container, stack.dropRef);
				s->LoadData();
				_inventory.emplace_back(std::move(s));
			}

			if (UserSettings::ShowIconBest()) {
				for (size_t index : Items::Inventory::FindBestInClassItems(inventory)) {
					_inventory[index]->GetData().bestInClass = true;
				}
			}
		}
	}

	void LootMenu::RefreshInventory(bool updateSelectedIndex)
	{
		PROFILE_SCOPE;

		_itemListProvider.ClearElements();

		const auto container = _container.get();
		if (!container) {
			_itemList.Invalidate();
			_itemList.SelectedIndex(-1.0);
			return;
		}

		LoadInventory();
		SortInventory();

		for (auto& item : _inventory) {
			_itemListProvider.PushBack(item->BuildDataObject(uiMovie.get()));
		}

		_itemList.InvalidateData();
		_lootMenu.GetInstance().Invoke("refresh");

		if (updateSelectedIndex) {
			SetSelectedIndex(_selectedIndex, false);
		}

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
			const auto index = keybinding.buttonArtOverride != Input::ButtonArtIndex::kNone ? keybinding.buttonArtOverride : Input::ButtonArt::GetFrameIndexForDeviceKey(keybinding.inputKey);

			RE::GFxValue obj;
			uiMovie->CreateObject(&obj);

			obj.SetMember("label", label);
			obj.SetMember("index", index);
			obj.SetMember("stolen", stealing);

			_buttonBarProvider.PushBack(obj);
		}

		bool isItemSelected = _selectedIndex >= 0 && _selectedIndex < _inventory.size();
		const auto entry = isItemSelected ? _inventory[_selectedIndex].get()->GetEntry() : nullptr;
		const auto dropRef = isItemSelected ? _inventory[_selectedIndex].get()->GetDropRef() : RE::ObjectRefHandle{};

		for (const auto& extraButton : API::APIServer::DispatchPopulateButtonBarEvent(_container, entry, dropRef)) {
			RE::GFxValue obj;
			uiMovie->CreateObject(&obj);

			obj.SetMember("label", extraButton.label.c_str());
			obj.SetMember("index", extraButton.buttonArtIndex);
			obj.SetMember("stolen", stealing);

			_buttonBarProvider.PushBack(obj);
		}

		_buttonBar.InvalidateData();
	}

	void LootMenu::RefreshInfoBar()
	{
		PROFILE_SCOPE;

		_infoBarProvider.ClearElements();

		if (_selectedIndex >= 0 && _selectedIndex < _inventory.size()) {
			const auto& selectedItem = _inventory[_selectedIndex];
			const auto strings = API::APIServer::DispatchPopulateInfoBarEvent(_container, selectedItem->GetEntry(), selectedItem->GetDropRef());

			for (auto string : strings) {
				if (!string.empty()) {
					_infoBarProvider.PushBack(string.c_str());
				}
			}
		}

		_infoBar.InvalidateData();
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
			std::string name = container->GetDisplayFullName();

			if (Integrations::NPCsNamesDistributor::IsReady()) {
				if (const auto actor = skyrim_cast<RE::Actor*>(container.get())) {
					name = Integrations::NPCsNamesDistributor::GetName(actor);
				}
			}

			_title.HTMLText(name);
			_title.Visible(true);
		}
	}

	const char* LootMenu::GetActionDisplayName(Input::QuickLootAction action, bool stealing) const
	{
		struct ActionDefinition
		{
			const char* labelKey;
			const char* labelFallback;

			const char* stealingLabelKey;
			const char* stealingLabelFallback;
		};

		ActionDefinition button;

		switch (action) {
		case Input::QuickLootAction::kTake:
			button = { "sTake", "Take", "sSteal", "Steal" };
			break;

		case Input::QuickLootAction::kTakeAll:
			button = { "sTakeAll", "Take All", "sTakeAll", "Take All" };
			break;

		case Input::QuickLootAction::kTransfer:
			button = { "sSearch", "Search", "sStealFrom", "Steal From" };
			break;

		case Input::QuickLootAction::kUse:
			{
				const auto selectedItem = _selectedIndex >= 0 && _selectedIndex < _inventory.size() ? _inventory[_selectedIndex].get() : nullptr;
				const char* useLabel = selectedItem ? selectedItem->GetUseLabel() : "$Use";

				button = { useLabel, useLabel, useLabel, useLabel };
				break;
			}

		default:
			return "<invalid>";
		}

		const auto labelKey = stealing ? button.stealingLabelKey : button.labelKey;
		const auto labelFallback = stealing ? button.stealingLabelFallback : button.labelFallback;

		const auto setting = RE::GameSettingCollection::GetSingleton()->GetSetting(labelKey);
		const auto label = setting ? setting->GetString() : labelFallback;

		return label;
	}

	bool LootMenu::CanDisplay(const RE::TESBoundObject& object)
	{
		if (!object.GetPlayable()) {
			return false;
		}

		auto name = object.GetName();
		if (!name || name[0] == '\0') {
			return false;
		}

		switch (object.GetFormType()) {
		case RE::FormType::Scroll:
		case RE::FormType::Armor:
		case RE::FormType::Book:
		case RE::FormType::Ingredient:
		case RE::FormType::Misc:
		case RE::FormType::Weapon:
		case RE::FormType::KeyMaster:
		case RE::FormType::AlchemyItem:
		case RE::FormType::Note:
		case RE::FormType::SoulGem:
			return true;

		case RE::FormType::Ammo:
			return !skyrim_cast<const RE::BGSKeywordForm*>(&object)->HasKeyword(Items::KnownForms::boundArrow.LookupForm<RE::BGSKeyword>());

		case RE::FormType::Light:
			return skyrim_cast<const RE::TESObjectLIGH*>(&object)->CanBeCarried();

		default:
			return false;
		}
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
		const float dx = UserSettings::VrOffsetX();
		const float dy = UserSettings::VrOffsetY();
		const float dz = UserSettings::VrOffsetZ();

		const float rx = static_cast<float>(UserSettings::VrAngleX() * std::numbers::pi / 180);
		const float ry = static_cast<float>(UserSettings::VrAngleY() * std::numbers::pi / 180);
		const float rz = static_cast<float>(UserSettings::VrAngleZ() * std::numbers::pi / 180);

		const float scale = UserSettings::VrScale();

		menuNode->local.translate = RE::NiPoint3(dx, dy, dz);
		menuNode->local.rotate.EulerAnglesToAxesZXY(rx, ry, rz);
		menuNode->local.scale = scale;

		GetAttachingNode()->AttachChild(menuNode.get(), true);
		RE::NiUpdateData data{};
		menuNode->Update(data);
	}

	void LootMenu::PostCreate()
	{
		if (!REL::Module::IsVR()) {
			return;
		}

		SetupMenuNode();
		SetTransform();

		auto def = uiMovie->GetMovieDef();
		auto rect = def->GetFrameRect();
		logger::info("movie: size: {}/{}, frame: {}/{}/{}/{}", def->GetWidth(), def->GetHeight(), rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);

		RE::GViewport viewport;
		uiMovie->GetViewport(&viewport);
		logger::info("viewport: buffer: {}/{}, origin: {}/{}, size: {}/{}", viewport.bufferWidth, viewport.bufferHeight, viewport.left, viewport.top, viewport.width, viewport.height);

		//viewport.bufferWidth = static_cast<int>(def->GetWidth());
		//viewport.bufferHeight = static_cast<int>(def->GetHeight());
		viewport.width = static_cast<int>(def->GetWidth());
		viewport.height = static_cast<int>(def->GetHeight());
		viewport.left = 0;
		viewport.top = 0;
		uiMovie->SetViewport(viewport);
		logger::info("viewport: buffer: {}/{}, origin: {}/{}, size: {}/{}", viewport.bufferWidth, viewport.bufferHeight, viewport.left, viewport.top, viewport.width, viewport.height);
	}

	RE::NiNode* LootMenu::GetMenuParentNode()
	{
		// No need to specify this here because we attach the node manually in SetTransform
		return nullptr;
	}

	RE::BSEventNotifyControl LootMenu::ProcessEvent(const RE::HudModeChangeEvent*, RE::BSTEventSource<RE::HudModeChangeEvent>*)
	{
		return RE::BSEventNotifyControl::kContinue;
	}

#pragma endregion
}
