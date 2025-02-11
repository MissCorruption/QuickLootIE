class QuickLoot.ListItemRenderer extends gfx.controls.ListItemRenderer
{
	/* PRIVATE VARIABLES */

	private static var STEALING_TEXT_COLOR: Number = 0xEF9A9A;
	private static var DEFAULT_TEXT_COLOR: Number = 0xFFFFFF;
	
	private static var DEFAULT_ICON_SOURCE = "skyui/icons_item_psychosteve.swf";
	private static var DEFAULT_ICON_LABEL = "default_misc";
	private static var DEFAULT_ICON_COLOR = 0xFFFFFF;
	private static var ICON_SPACING = 3;
	private static var ICON_SIZE = 24;
	
	private var _lootMenu: QuickLoot.LootMenu;
	
	private var _iconLoader: MovieClipLoader;
	private var _iconSource: String = DEFAULT_ICON_SOURCE;
	private var _iconLabel: String = DEFAULT_ICON_LABEL;
	private var _iconColor: Number = undefined;
	
	private var _selectedIcons: Array = [];
	private var _totalIconWidth: Number = 0;
	private var _totalColumnWidth: Number = 0;
	private var _isTextTrimmed: Boolean = false;
	
	private var _hasData: Boolean = false;

	/* STAGE ELEMENTS */

	public var itemIcon: MovieClip;
	public var itemName: TextField;
	
	public var stolenIcon: MovieClip;
	public var readIcon: MovieClip;
	
	public var enchantIcon: MovieClip;
	public var knownEnchantIcon: MovieClip;
	public var specialEnchantIcon: MovieClip;
	
	public var dbmNew: MovieClip;
	public var dbmFound: MovieClip;
	public var dbmDisplayed: MovieClip;
	
	public var compNew: MovieClip;
	public var compFound: MovieClip;
	
	public var weightText: TextField;
	public var valueText: TextField;
	public var valuePerWeightText: TextField;
	
	public function ListItemRenderer(a_obj: Object)
	{
		super();
		
		_lootMenu = _root.lootMenu;
		
		_iconLoader = new MovieClipLoader();
		_iconLoader.addListener(this);
		_iconLoader.loadClip(_iconSource, itemIcon);
		
		reset();
	}
	
	public function reset()
	{
		_visible = false;
		_hasData = false;
		
		itemIcon._visible = false;
		itemName._visible = false;
		
		stolenIcon._visible = false;
		readIcon._visible = false;
		
		enchantIcon._visible = false;
		knownEnchantIcon._visible = false;
		specialEnchantIcon._visible = false;
		
		dbmNew._visible = false;
		dbmFound._visible = false;
		dbmDisplayed._visible = false;
		
		compNew._visible = false;
		compFound._visible = false;
		
		weightText._visible = false;
		valueText._visible = false;
		valuePerWeightText._visible = false;
	}
	
	public function setData(data: Object): Void
	{
		if (!data) return;
		
		_visible = true;
		_hasData = true;

		// Call i4 if it is installed
		skse.plugins.InventoryInjector.ProcessEntry(data);
		
		if(_lootMenu.showItemIcons) {
			// Do this first, so the icon source can load
			// in the background while we initialize the rest.
			setItemIcon(data.iconSource, data.iconLabel, data.iconColor);
			itemIcon._visible = true;
			itemName._x = itemIcon._x + itemIcon._width + ICON_SPACING;
		}
		else {
			itemIcon._visible = false;
			itemName._x = itemIcon._x;
		}
		
		// Data
		
		var displayName: String = data.displayName;
		var count: Number = data.count;
		var stolen: Boolean = data.stolen;
		var weight: Number = data.weight;
		var value: Number = data.value;
		
		// Column values
		
		arrangeInfoColumns();
		updateColumnValue(valueText, value, 2);
		updateColumnValue(weightText, weight, 2);
		updateColumnValue(valuePerWeightText, value / weight, 0);
		
		// Item name and trailing icons
		
		selectIcons(data);
		updateItemName(displayName, data.textColor, count, stolen);
		arrangeIcons();
	}
	
	private function arrangeInfoColumns() {
		_totalColumnWidth = 0;
		
		for(var i = _lootMenu.infoColumns.length - 1; i >= 0; i--) {
			var columnName = _lootMenu.infoColumns[i];
			var element = this[columnName + "Text"];
			
			if(typeof(element) == "object") {
				element._visible = true;
				_totalColumnWidth = _totalColumnWidth + element._width;
				element._x = this._width - _totalColumnWidth;
			}
		}
	}
	
	private function updateColumnValue(text: TextField, value: Number, precision: Number)
	{
		if(!value || isNaN(value) || !isFinite(value)) {
			text.text = "-";
			return;
		}
		
		text.text = formatNumber(value, precision);
	}
	
	public function updateItemName(displayName: String, color: Number, count: Number, stealing: Boolean)
	{
		if(!displayName) displayName = "<unnamed>";
		if(!color) color = stealing ? STEALING_TEXT_COLOR : DEFAULT_TEXT_COLOR;
		if(!count) count = 1;
		
		var textWidth = this._width - _totalColumnWidth - _totalIconWidth - itemName._x;
		itemName.text = trimItemName(displayName, count, itemName.getTextFormat(), textWidth);
		itemName.autoSize = "left";
		itemName.wordWrap = false;
		itemName.textColor = color;
		itemName._visible = true;
	}
	
	private function trimItemName(name: String, count: Number, format: TextFormat, maxWidth: Number)
	{
		var text = name;
		if(count > 1) text += " (" + count + ")";
		
		_isTextTrimmed = false;
		
		var width = format.getTextExtent(text).textFieldWidth;
		if(width <= maxWidth) return text;
		
		_isTextTrimmed = true;
		
		var chars = text.length - 1;
		var suffix = "...";
		if(count > 1) suffix += " (" + count + ")";
		
		while(chars > 0) {
			text = text.substr(0, chars) + suffix;
			
			var width = format.getTextExtent(text).textFieldWidth;
			if(width <= maxWidth) return text;
			
			chars--;
			
			// Don't add an ellipsis after whitespace or punctuation.
			while(chars > 0 && " .!?'()".indexOf(text.charAt(chars - 1)) >= 0) {
				chars--;
			}
		}
		
		// This should be unreachable.
		return text;
	}
	
	private function selectIcons(data: Object)
	{
		_selectedIcons = [];
		_totalIconWidth = 0;
		
		selectIcon(stolenIcon, data.stolen);
		selectIcon(readIcon, data.read);
		
		selectIcon(knownEnchantIcon, data.knownEnchanted)
		|| selectIcon(specialEnchantIcon, data.specialEnchanted)
		|| selectIcon(enchantIcon, data.enchanted);
		
		selectIcon(dbmNew, data.dbmNew)
		|| selectIcon(dbmDisplayed, data.dbmDisplayed)
		|| selectIcon(dbmFound, data.dbmFound);
		
		selectIcon(compNew, data.compNew)
		|| selectIcon(compFound, data.compFound);
	}
	
	function selectIcon(icon: MovieClip, enable: Boolean)
	{
		if(!enable) return false;
		
		icon._visible = true;
		_selectedIcons.push(icon);
		_totalIconWidth += icon._width + ICON_SPACING;
		return true;
	}
	
	private function arrangeIcons()
	{
		var x = _isTextTrimmed
			? this._width - _totalColumnWidth - _totalIconWidth
			: itemName._x + itemName._width;
		
		// Using a for in loop here iterates in reverse index order for some reason.
		for(var i = 0; i < _selectedIcons.length; i++) {
			var icon = _selectedIcons[i];
			
			icon._x = x + ICON_SPACING
			x += ICON_SPACING + icon._width;
		}
	}
	
	private function formatNumber(number: Number, precision: Number)
	{
 		var factor = Math.pow(10, precision);
 		return (Math.round(number * factor) / factor).toString();
	}
	
	private function setItemIcon(iconSource: String, iconLabel: String, iconColor: Number)
	{
		if(!iconSource) iconSource = DEFAULT_ICON_SOURCE;
		if(!iconLabel) iconLabel = DEFAULT_ICON_LABEL;
		
		var iconSourceChanged = iconSource != _iconSource;
		
		_iconSource = iconSource;
		_iconLabel = iconLabel;
		_iconColor = iconColor;
		
		// Either trigger an asynchronous swf load or invoke the callback directly
		if(iconSourceChanged) {
			_iconLoader.loadClip(_iconSource, itemIcon);
		}
		else {
			onLoadInit(itemIcon);
		}
	}
	
	// This is called when the _iconLoader has finished loading the icon source swf
	private function onLoadInit(icon: MovieClip): Void
	{
		icon.gotoAndStop(_iconLabel);
		icon._width = itemIcon._height = ICON_SIZE;
		
		if(!_hasData || !_lootMenu.showItemIcons) {
			icon._visible = false;
		}
		
		var colorTransform = new flash.geom.ColorTransform();
		if(typeof(_iconColor) == "number") {
			colorTransform.rgb = _iconColor;
		}
		
		var transform = new flash.geom.Transform(MovieClip(icon));
		transform.colorTransform = colorTransform;
	}
}
