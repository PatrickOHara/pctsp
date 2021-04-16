#ifndef __PCTSP_LOGGER__
#define __PCTSP_LOGGER__
#include <iostream>

#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/expressions.hpp>

namespace logging = boost::log;

enum PyLoggingLevels {
    CRITICAL = 50,
    ERROR = 40,
    WARNING = 30,
    INFO = 20,
    DEBUG = 10,
    NOTSET = 0,
};

int getBoostLevelFromPyLevel(int py_logging_level) ;

void PCTSPinitLogging(int level);
#endif