#ifndef LOGGER_HELPER_H
#define LOGGER_HELPER_H

#include "engine/io/iowning_glogger.h"
#include <spdlog/spdlog.h>
inline LOG_LEVEL spd_to_glog(spdlog::level::level_enum level)
{
	return (LOG_LEVEL)level;
}

inline spdlog::level::level_enum glog_to_spd(LOG_LEVEL level)
{
	return (spdlog::level::level_enum)level;
}


#endif // LOGGER_HELPER_H