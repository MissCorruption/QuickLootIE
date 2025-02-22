#include "Profiler.h"

#include "Config/SystemSettings.h"

#include <spdlog/sinks/basic_file_sink.h>

namespace QuickLoot::Util
{
	IntervalHandle::~IntervalHandle()
	{
		Profiler::EndInterval(*this);
	}

	void Profiler::Init()
	{
		const auto plugin = SKSE::PluginDeclaration::GetSingleton();
		
		auto path = logger::log_directory();
		if (!path) {
			SKSE::stl::report_and_fail("Failed to find standard logging directory"sv);
		}

		*path /= std::format("{}_Profiler.log"sv, plugin->GetName());

		_logger = std::make_unique<spdlog::logger>("profiler");

		if (!Config::SystemSettings::EnableProfiler()) {
			std::filesystem::remove(*path);
			logger::info("Profiler is disabled");
			return;
		}

		const int interval = Config::SystemSettings::ProfilerFlushInterval();
		if (interval == 0) {
			_logger->flush_on(spdlog::level::info);
		} else {
			_flusher = std::make_unique<spdlog::details::periodic_worker>([] { _logger->flush(); }, std::chrono::milliseconds(interval));
		}

		const auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
		_logger->sinks().push_back(sink);
		_logger->set_pattern("[%H:%M:%S.%e] [%t] %v");
		_logger->set_level(spdlog::level::info);

		logger::info("Started profiler");
	}

	std::string indent(size_t depth)
	{
		static std::string indentCache = "| | | | | | | | ";

		while (depth > indentCache.length() / 2) {
			indentCache += "| ";
		}

		return indentCache.c_str() + indentCache.length() - depth * 2;
	}

	IntervalHandle Profiler::StartInterval(ScopeId scope)
	{
		if (!Config::SystemSettings::EnableProfiler()) {
			return -1;
		}

		if (!_intervalStack.empty() && !_loggedStart) {
			const auto& interval = _intervalStack[_intervalStack.size() - 1];
			_logger->info("{}{} started", indent(_intervalStack.size() - 1), _scopeNames[interval.scope]);
		}

		const auto id = _nextIntervalId++;

		_loggedStart = false;
		_intervalStack.push_back({ id, scope, profiler_clock::now() });

		return { id };
	}

	void Profiler::EndInterval(const IntervalHandle& handle)
	{
		if (!Config::SystemSettings::EnableProfiler()) {
			return;
		}

		if (_intervalStack.empty() || _intervalStack[_intervalStack.size() - 1].id != handle.id) {
			logger::warn("Unbalanced profiler intervals");
		}

		const auto& interval = _intervalStack[_intervalStack.size() - 1];
		const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(profiler_clock::now() - interval.startTime);

		_logger->info("{}{} took {}", indent(_intervalStack.size() - 1), _scopeNames[interval.scope], duration);
		_loggedStart = true;
		_intervalStack.pop_back();
	}

	ScopeId Profiler::RegisterScope(const std::string& name)
	{
		if (!Config::SystemSettings::EnableProfiler()) {
			return 0;
		}

		std::lock_guard lock(_mutex);

		const auto it = std::ranges::find(_scopeNames, name);
		if (it != _scopeNames.end()) {
			return static_cast<ScopeId>(it - _scopeNames.begin());
		}

		_scopeNames.push_back(name);
		return static_cast<ScopeId>(_scopeNames.size() - 1);
	}
}
