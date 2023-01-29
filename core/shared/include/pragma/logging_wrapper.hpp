/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __PRAGMA_LOGGING_WRAPPER_HPP__
#define __PRAGMA_LOGGING_WRAPPER_HPP__

#include "pragma/networkdefinitions.h"
#include <optional>
#include <sharedutils/util_log.hpp>

namespace pragma {
	constexpr auto DEFAULT_CONSOLE_LOG_LEVEL = ::util::LogSeverity::Warning;
	constexpr auto DEFAULT_FILE_LOG_LEVEL = ::util::LogSeverity::Info;
	constexpr auto DEFAULT_LOG_FILE = "log.txt";
	namespace logging {
		DLLNETWORK int32_t severity_to_spdlog_level(::util::LogSeverity severity);
	};
	DLLNETWORK void log(const std::string &msg, ::util::LogSeverity severity = ::util::LogSeverity::Info);
	DLLNETWORK bool is_log_level_enabled(::util::LogSeverity severity);

	namespace detail {
		DLLNETWORK void initialize_logger(::util::LogSeverity conLogLevel = DEFAULT_CONSOLE_LOG_LEVEL, ::util::LogSeverity fileLogLevel = DEFAULT_FILE_LOG_LEVEL, const std::optional<std::string> &logFile = DEFAULT_LOG_FILE);
		DLLNETWORK void close_logger();
		DLLNETWORK std::optional<std::string> get_log_file_name();
	};
};

#endif
