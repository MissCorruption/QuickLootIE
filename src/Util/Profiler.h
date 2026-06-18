#pragma once

#include "Config/SystemSettings.h"

#define PROFILE_MERGE2(a, b) a##b
#define PROFILE_MERGE(a, b) PROFILE_MERGE2(a, b)
#define PROFILE_UNIQUE(name) PROFILE_MERGE(name, __LINE__)

#define PROFILE_SCOPE PROFILE_SCOPE_NAMED(__FUNCTION__)
#define PROFILE_SCOPE_NAMED(name)                                                               \
	static const auto PROFILE_UNIQUE(scope) = ::QuickLoot::Util::Profiler::RegisterScope(name); \
	const auto PROFILE_UNIQUE(interval) = ::QuickLoot::Util::Profiler::StartInterval(PROFILE_UNIQUE(scope));

#define PROFILE_LOG(...) ::QuickLoot::Util::Profiler::Log(__VA_ARGS__);

namespace QuickLoot::Util
{
	using profiler_clock = std::chrono::high_resolution_clock;
	using ScopeId = int;
	using IntervalId = int;

	struct IntervalHandle
	{
		IntervalHandle(IntervalHandle&&) = delete;
		IntervalHandle(const IntervalHandle&) = delete;
		IntervalHandle& operator=(IntervalHandle&&) = delete;
		IntervalHandle& operator=(const IntervalHandle&) = delete;

		IntervalHandle(IntervalId id) :
			id(id) {}

		~IntervalHandle();

		IntervalId id;
	};

	class Profiler
	{
	public:
		Profiler() = delete;
		~Profiler() = delete;
		Profiler(Profiler&&) = delete;
		Profiler(const Profiler&) = delete;
		Profiler& operator=(Profiler&&) = delete;
		Profiler& operator=(const Profiler&) = delete;

		static void Init();

		static ScopeId RegisterScope(const std::string& name);

		static IntervalHandle StartInterval(ScopeId scope);
		static void EndInterval(const IntervalHandle& handle);

		template <typename... Args>
		static void Log(fmt::format_string<Args...> format, Args&&... args)
		{
			if (!Config::SystemSettings::EnableProfiler()) {
				return;
			}

			_logger->info("{}{}", GetIndent(_intervalStack.size() - 1), fmt::format(format, std::forward<Args>(args)...));
		}

	private:
		static std::string GetIndent(size_t depth);

		struct Interval
		{
			IntervalId id;
			ScopeId scope;
			profiler_clock::time_point startTime;
		};

		static inline std::unique_ptr<spdlog::logger> _logger;
		static inline std::mutex _mutex{};
		static inline std::vector<std::string> _scopeNames{ "" };
		static inline thread_local IntervalId _nextIntervalId{};
		static inline thread_local bool _loggedStart = false;
		static inline thread_local std::vector<Interval> _intervalStack{};
		static inline std::unique_ptr<spdlog::details::periodic_worker> _flusher;
	};
}
