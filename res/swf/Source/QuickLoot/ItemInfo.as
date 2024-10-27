class QuickLoot.ItemInfo extends gfx.controls.Button
{
	/* INITIALIZATION */

	// @override gfx.controls.Button
	public function ItemInfo()
	{
		super();
	}


	/* PRIVATE FUNCTIONS */

	// @override gfx.controls.Button
	private function configUI(): Void
	{
		/*
		disableConstraints = true;
		textField.autoSize = "left";
		textField._alpha = 90;

		super.configUI();
		constraints.scaled = false;
		*/
		textField._visible = false;
	}


	// @override gfx.controls.Button
	private function updateAfterStateChange(): Void
	{
		/*
		super.updateAfterStateChange();

		if (initialized) {
			setSize(textField.textWidth, textField.textHeight);
		}
		*/
	}
}
