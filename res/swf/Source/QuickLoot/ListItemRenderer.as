class QuickLoot.ListItemRenderer extends gfx.controls.ListItemRenderer
{
	/* PRIVATE VARIABLES */

	private static var STEALING_TEXT_COLOR: Number = 0xEF9A9A;
	private static var DEFAULT_TEXT_COLOR: Number = 0xFFFFFF;
	private static var MAX_TEXT_WIDTH: Number = 315;
	
	private static var DEFAULT_ICON_SOURCE = "skyui/icons_item_psychosteve.swf";
	private static var DEFAULT_ICON_LABEL = "default_misc";
	private static var DEFAULT_ICON_COLOR = 0xFFFFFF;
	private static var ICON_SPACING = 3;
	private static var ICON_SIZE = 24;
	
	private var _iconLoader: MovieClipLoader;
	private var _iconSource: String = DEFAULT_ICON_SOURCE;
	private var _iconLabel: String = DEFAULT_ICON_LABEL;
	private var _iconColor: Number = undefined;
	
	private var _selectedIcons: Array = [];
	private var _totalIconWidth: Number = 0;
	private var _isTextTrimmed: Boolean = false;

	/* STAGE ELEMENTS */

	public var textField: TextField;
	public var itemIcon: MovieClip;
	
	public var stolenIcon: MovieClip;
	public var readIcon: MovieClip;
	
	public var enchantIcon: MovieClip;
	public var knownEnchantIcon: MovieClip;
	public var specialEnchantIcon: MovieClip;
	
	public var dbmNew: MovieClip;
	public var dbmFound: MovieClip;
	public var dbmDisp: MovieClip;
	
	public var compNew: MovieClip;
	public var compFound: MovieClip;
	
	public var itemWeight: TextField;
	public var itemValue: TextField;
	public var itemValuePerWeight: TextField;
	
	public function ListItemRenderer(a_obj: Object)
	{
		super();
		
		_iconLoader = new MovieClipLoader();
		_iconLoader.addListener(this);
		_iconLoader.loadClip(_iconSource, itemIcon);
		
		reset();
	}
	
	public function reset()
	{
		itemIcon._visible = false;
		
		stolenIcon._visible = false;
		readIcon._visible = false;
		
		enchantIcon._visible = false;
		knownEnchantIcon._visible = false;
		specialEnchantIcon._visible = false;
		
		dbmNew._visible = false;
		dbmFound._visible = false;
		dbmDisp._visible = false;
		
		compNew._visible = false;
		compFound._visible = false;
		
		itemWeight._visible = false;
		itemValue._visible = false;
		itemValuePerWeight._visible = false;
	}
	
	public function setData(data: Object): Void
	{
		if (!data) return;

		// Call i4 if it is installed
		skse.plugins.InventoryInjector.ProcessEntry(data);
		
		// Do this first, so the icon source can load
		// in the background while we initialize the rest.
		setItemIcon(data.iconSource, data.iconLabel, data.iconColor);
		itemIcon._visible = true;
		
		// Data
		
		var displayName: String = data.displayName;
		var count: Number = data.count;
		var stolen: Boolean = data.stolen;
		var weight: Number = data.weight;
		var value: Number = data.value;
		
		// Column values
		
		updateColumnValue(itemValue, value, 2);
		updateColumnValue(itemWeight, weight, 2);
		updateColumnValue(itemValuePerWeight, value / weight, 0);
		
		// Item name and trailing icons
		
		selectIcons(data);
		updateItemName(displayName, data.textColor, count, stolen);
		arrangeIcons();
	}
	
	public function updateItemName(displayName: String, color: Number, count: Number, stealing: Boolean)
	{
		if(!displayName) displayName = "<unnamed>";
		if(!color) color = stealing ? STEALING_TEXT_COLOR : DEFAULT_TEXT_COLOR;
		if(!count) count = 1;
		
		label = trimItemName(displayName, count, textField.getTextFormat(), MAX_TEXT_WIDTH - _totalIconWidth);
		textField.autoSize = "left";
		textField.wordWrap = false;
		textField.textColor = color;
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
		|| selectIcon(dbmDisp, data.dbmDisp)
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
		var x = textField._x;
		x += _isTextTrimmed
			? MAX_TEXT_WIDTH - _totalIconWidth
			: textField._width;
		
		// Using a for in loop here iterates in reverse index order for some reason.
		for(var i = 0; i < _selectedIcons.length; i++) {
			var icon = _selectedIcons[i];
			
			icon._x = x + ICON_SPACING
			x += ICON_SPACING + icon._width;
		}
	}
	
	private function updateColumnValue(text: TextField, value: Number, precision: Number)
	{
		if(!value || isNaN(value) || !isFinite(value)) {
			text.text = "-";
			text._visible = true;
			return;
		}
		
		text.text = formatNumber(value, precision);
		text._visible = true;
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
		
		var colorTransform = new flash.geom.ColorTransform();
		if(typeof(_iconColor) == "number") {
			colorTransform.rgb = _iconColor;
		}
		
		var transform = new flash.geom.Transform(MovieClip(icon));
		transform.colorTransform = colorTransform;
	}
}
