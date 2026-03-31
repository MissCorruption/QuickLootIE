#include "ScaleformUtil.h"
#include "FormUtil.h"

namespace QuickLoot::Util
{
	RE::GFxValue ScaleformUtil::BuildGFxValueFromJson(RE::GFxMovieView* view, const json& value)
	{
		switch (value.type()) {
		case nlohmann::detail::value_t::null:
			return { RE::GFxValue::ValueType::kNull };

		case nlohmann::detail::value_t::object:
			{
				RE::GFxValue obj{};
				view->CreateObject(&obj);

				for (auto [key, val] : value.items()) {
					obj.SetMember(key.c_str(), BuildGFxValueFromJson(view, val));
				}

				return obj;
			}

		case nlohmann::detail::value_t::array:
			{
				RE::GFxValue arr{};
				view->CreateArray(&arr);

				for (auto [_, val] : value.items()) {
					arr.PushBack(BuildGFxValueFromJson(view, val));
				}

				return arr;
			}

		case nlohmann::detail::value_t::string:
			{
				RE::GFxValue str{};
				view->CreateString(&str, value.get<std::string>().c_str());
				return str;
			}

		case nlohmann::detail::value_t::boolean:
			return { value.get<bool>() };

		case nlohmann::detail::value_t::number_integer:
			return { value.get<int64_t>() };

		case nlohmann::detail::value_t::number_unsigned:
			return { value.get<uint64_t>() };

		case nlohmann::detail::value_t::number_float:
			return { value.get<double>() };

		default:
			return { RE::GFxValue::ValueType::kUndefined };
		}
	}
}
