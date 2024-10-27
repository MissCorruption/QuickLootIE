#pragma once

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
#pragma warning(disable: 4702)
#define SKSE_SUPPORT_XBYAK
#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"
#include <xbyak/xbyak.h>
#pragma warning(pop)

constexpr auto LEGACY_SCALEFORM { "Data\\Interface\\LootMenu.swf" };
constexpr auto SCALEFORM { "Data\\Interface\\LootMenuIE.swf" };

using namespace std::literals::string_view_literals;

namespace logger = SKSE::log;

#include "Plugin.h"

#include <ClibUtil/distribution.hpp>
#include <ClibUtil/editorID.hpp>
#include <ClibUtil/numeric.hpp>
#include <ClibUtil/rng.hpp>
#include <ClibUtil/simpleINI.hpp>

#include <SimpleMath.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;
