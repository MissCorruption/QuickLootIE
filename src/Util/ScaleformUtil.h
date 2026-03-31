#pragma once

namespace QuickLoot::Util
{
	class ScaleformUtil
	{
	public:
		ScaleformUtil() = delete;
		~ScaleformUtil() = delete;
		ScaleformUtil(ScaleformUtil const&) = delete;
		ScaleformUtil(ScaleformUtil const&&) = delete;
		ScaleformUtil& operator=(ScaleformUtil&) = delete;
		ScaleformUtil& operator=(ScaleformUtil&&) = delete;

		static RE::GFxValue BuildGFxValueFromJson(RE::GFxMovieView* view, const json& value);
	};
}
