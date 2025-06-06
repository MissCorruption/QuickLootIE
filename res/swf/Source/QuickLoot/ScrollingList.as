﻿class QuickLoot.ScrollingList extends gfx.controls.ScrollingList
{
	public function canScrollUp(): Boolean
	{
		return scrollPosition > 0;
	}
	
	public function canScrollDown(): Boolean
	{
		//QuickLoot.Utils.log("scroll: " + scrollPosition + ", count: " + _dataProvider.length + ", height: " + height + ", row height: " + _rowHeight);
		return scrollPosition < _dataProvider.length - height / _rowHeight;
	}
	
	private function scrollToIndex(value: Number): Void
	{
		super.scrollToIndex(value);
		this.dispatchEvent({ type: "scrollPositionChanged", scrollPosition: scrollPosition });
	}
	
	private function populateData(data: Array): Void
	{
		for (var i: Number = 0; i < totalRenderers; i++) {
			var renderer: MovieClip = renderers[i];
			renderer.reset();
		}
		super.populateData(data);
	}
}
