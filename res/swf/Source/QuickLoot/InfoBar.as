class QuickLoot.InfoBar extends gfx.controls.ButtonBar
{
	/* INITIALIZATION */

	// @override ButtonBar
	public function InfoBar()
	{
		super();
	}


	/* PUBLIC FUNCTIONS */

	// @override gfx.controls.ButtonBar
	public function invalidateData(): Void
	{
		reflowing = false;
		invalidate();
	}


	/* PRIVATE FUNCTIONS */

	// @override gfx.core.UIComponent
	private function draw(): Void
	{
		if (!reflowing) {
			// Update current buttons
			var l: Number = _dataProvider.length;
			while (renderers.length > l) {
				var r: MovieClip = MovieClip(renderers.pop());
				r.group.removeButton(r);
				r.removeMovieClip();
			}

			while (renderers.length < l) {
				renderers.push(createRenderer(renderers.length));
			}

			populateData();
			reflowing = true;
		}

		if (drawLayout() && _selectedIndex != -1) {
			selectItem(_selectedIndex);
		}
	}


	/**
	 * @override gfx.controls.ButtonBar
	 * implements a horizontal, right-aligned layout
	 */
	private function drawLayout(): Boolean
	{
		// If the (last) renderer is not yet ready, invalidate to force a redraw.
		if (renderers.length > 0 && !renderers[renderers.length - 1].initialized) {
			reflowing = true;
			invalidate();
			return false;
		}
		reflowing = false;

		var calcY: Function = function(a_height: Number): Number {
			return Math.max((this.height - a_height) / 2, 0);
		};

		var pos: Number = width;
		for (var i: Number = 0; i < renderers.length; ++i) {
			var renderer: MovieClip = renderers[i];

			pos -= renderer.width;
			renderer._x = pos;
			pos -= _spacing;

			renderer._y = calcY(renderer.height);
			renderer._visible = true;
		}

		return true;
	}


	// @override gfx.controls.ButtonBar
	private function createRenderer(a_index: Number): MovieClip
	{
		var renderer: MovieClip = super.createRenderer(a_index);
		renderer._visible = false;	// avoid visible snapping into place
		return renderer;
	}
}
