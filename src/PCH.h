#pragma once

// Visual Studio refuses to say what's actually causing the error, so let's just disable it globally
// I'm sure that's a good idea
#define _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <bitset>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <initializer_list>
#include <limits>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <variant>
#include <vector>

#pragma warning(push)
#include <frozen/map.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <srell.hpp>

#include <spdlog/sinks/basic_file_sink.h>
#pragma warning(pop)

namespace logger = SKSE::log;
namespace WinAPI = SKSE::WinAPI;

using namespace std::literals;

namespace stl
{
	using namespace SKSE::stl;

	constexpr std::string_view safe_string(const char* a_str) { return a_str ? a_str : ""sv; }
}

#define DLLEXPORT __declspec(dllexport)

#include "Plugin.h"

#include "Settings.h"
