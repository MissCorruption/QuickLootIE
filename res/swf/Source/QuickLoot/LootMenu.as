class QuickLoot.LootMenu extends gfx.core.UIComponent
{
	private static var HEIGHT_PER_LINE = 40;
	private static var NEUTRAL_LINE_COUNT = 7;
	private static var NEUTRAL_WIDTH = 546;
	private static var NEUTRAL_HEIGHT = 371;
	
	private static var COLUMNS_BASE_X = NEUTRAL_WIDTH - 23;
	
	// stage elements
	
	private var itemList: QuickLoot.ScrollingList;
	private var title: TextField;
	private var weight: TextField;
	private var infoBar: QuickLoot.InfoBar;
	private var buttonBar: QuickLoot.ButtonBar;
	
	private var arrowUp: MovieClip;
	private var arrowDown: MovieClip;
	
	private var valueHeader: TextField;
	private var weightHeader: TextField;
	private var valuePerWeightHeader: TextField;
	
	private var background: MovieClip;
	
	// private variables
	
	private var columnHeaders: Array;
	private var movingElements: Array;
	private var nonTransparentElements: Array;
	
	private var stageCenterX: Number;
	private var stageCenterY: Number;
	
	// settings
	
	public var minLines = 3;
	public var maxLines = 10;
	
	public var offsetX = 100;
	public var offsetY = -200;
	public var scale = 1;
	
	public var alphaNormal = 100;
	public var alphaEmpty = 30;
	
	public var anchorFractionX = 0;
	public var anchorFractionY = 0;
	
	public var infoColumns = ["value", "weight", "valuePerWeight"];
	
	public var showItemIcons = true;
	
	// public functions
	
	public function getVersion() {
		/*
		Feature levels:
		1: supports getVersion()
		2: supports configurable info columns
		3: supports showItemIcons (+bugfix)
		*/
		return 3;
	}
	
	public function init(settings: Object)
	{
		//QuickLoot.Utils.log("Initializing LootMenu");
		
		loadSetting(settings, "minLines", "number");
		loadSetting(settings, "maxLines", "number");
		
		loadSetting(settings, "offsetX", "number");
		loadSetting(settings, "offsetY", "number");
		loadSetting(settings, "scale", "number");
		
		loadSetting(settings, "alphaNormal", "number");
		loadSetting(settings, "alphaEmpty", "number");
		
		loadSetting(settings, "anchorFractionX", "number");
		loadSetting(settings, "anchorFractionY", "number");
		
		loadSetting(settings, "infoColumns", "object");
		
		loadSetting(settings, "showItemIcons", "boolean");
		
		if(scale == 0) scale = 1;
		
		// The CoreList constructor sets a scale9Grid, which causes very odd
		// behavior when changing the list size after it's created.
		itemList["container"].scale9Grid = null;
		itemList.rowCount = maxLines;
		var self = this;
		itemList.addEventListener("scrollPositionChanged", function() { self.updateScrollArrows(); });
		
		columnHeaders = [valueHeader, weightHeader, valuePerWeightHeader];
		movingElements = [weight, infoBar, buttonBar, arrowDown];
		nonTransparentElements = [buttonBar];
		
		saveInitialElementBounds();
		initColumnHeaders()
		refresh();
	}
	
	public function refresh()
	{
		var lineCount = itemList.dataProvider.length;
		var isEmpty = lineCount == 0;
		
		resizeContainer(lineCount);
		setOpacity(isEmpty ? alphaEmpty : alphaNormal);
		updateScale();
		updateScrollArrows();
	}
	
	// private functions
	
	private function loadSetting(settings: Object, name: String, type: String)
	{
		//QuickLoot.Utils.log(name + " (" + type + "): " + settings[name] + " (" + typeof(settings[name]) + ")");
		
		if(typeof(settings[name]) == type) {
			this[name] = settings[name];
		}
	}
	
	private function initColumnHeaders()
	{
		for(var i in columnHeaders) {
			var element = columnHeaders[i];
			element._visible = false;
		}
		
		var x = COLUMNS_BASE_X;
		for(var i = infoColumns.length - 1; i >= 0; i--) {
			var columnName = infoColumns[i];
			var element = this[columnName + "Header"];
			if(typeof(element) == "object") {
				element._visible = true;
				x = x - element._width;
				element._x = x;
			}
		}
	}
	
	private function resizeContainer(lineCount: Number)
	{
		lineCount = Math.min(lineCount, maxLines);
		lineCount = Math.max(lineCount, minLines);
		
		var shiftAmount = (lineCount - NEUTRAL_LINE_COUNT) * HEIGHT_PER_LINE;
		
		background._height = background._originalH + shiftAmount;
		
		// Flash and Scaleform both have a bug where the transform origin of a MovieClip isn't
		// calculated correctly when 9-slice scaling is used. That causes the background clip to
		// shift around if its local bounds extend into negative coordinates.
		
		var bgBounds = background.getBounds(background);
		var yMin = bgBounds.yMin, yMax = bgBounds.yMax;
		var originFraction = -yMin / background._originalH;
		var originalOriginOffset = -yMin; // = background._originalH * originFraction;
		var currentOriginOffset = background._height * originFraction;
		background._y = currentOriginOffset - originalOriginOffset;
		
		for(var i in movingElements) {
			var element = movingElements[i];
			element._y = element._originalY + shiftAmount;
		}
	}
	
	private function updateScale()
	{
		var bounds = getBounds(this);
		var deltaWidth = background._width - background._originalW;
		var deltaHeight = background._height - background._originalH;
		var anchorX = (NEUTRAL_WIDTH + deltaWidth) * anchorFractionX;
		var anchorY = (NEUTRAL_HEIGHT + deltaHeight) * anchorFractionY;
		
		_width = (bounds.xMax - bounds.xMin) * scale;
		_height = (bounds.yMax - bounds.yMin) * scale;
		_x = stageCenterX + offsetX - anchorX * scale;
		_y = stageCenterY + offsetY - anchorY * scale;
	}
	
	private function updateScrollArrows()
	{
		arrowUp._visible = itemList.canScrollUp();
		arrowDown._visible = itemList.canScrollDown();
	}
	
	private function contains(array: Array, element) {
		for(var i in array) {
			if(array[i] == element) {
				return true;
			}
		}
		
		return false;
	}
	
	private function setOpacity(opacity: Number)
	{
		for(var member in this) {
			var element = this[member];
			
			if(!(element instanceof MovieClip) && !(element instanceof TextField)) continue;
			if(contains(nonTransparentElements, element)) {
				continue;
			}
			
			element._alpha = opacity * 100;
		}
	}
	
	private function saveInitialElementBounds()
	{
		stageCenterX = _x;
		stageCenterY = _y;
		
		for(var member in this) {
			var element = this[member];
			
			if(!(element instanceof MovieClip) && !(element instanceof TextField)) continue;
			
			//QuickLoot.Utils.log(member + ": " + element._x + ", " + element._y + ", " + element._width + ", " + element._height);
			
			element._originalX = element._x;
			element._originalY = element._y;
			element._originalW = element._width;
			element._originalH = element._height;
		}
	}
}