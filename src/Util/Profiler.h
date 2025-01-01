#pragma once

#define PROFILE_MERGE2(a, b) a##b
#define PROFILE_MERGE(a, b) PROFILE_MERGE2(a, b)
#define PROFILE_UNIQUE(name) PROFILE_MERGE(name, __LINE__)

#define PROFILE_SCOPE PROFILE_SCOPE_NAMED(__FUNCTION__)
#define PROFILE_SCOPE_NAMED(name)                                                               \
	static const auto PROFILE_UNIQUE(scope) = ::QuickLoot::Util::Profiler::RegisterScope(name); \
	const auto PROFILE_UNIQUE(interval) = ::QuickLoot::Util::Profiler::StartInterval(PROFILE_UNIQUE(scope));

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

	private:
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
