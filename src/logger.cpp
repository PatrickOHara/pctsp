
#include "pctsp/logger.hh"

int getBoostLevelFromPyLevel(int py_logging_level) {
    int boost_level;
    switch (py_logging_level) {
    case CRITICAL:
        boost_level = logging::trivial::fatal;
        break;
    case ERROR:
        boost_level = logging::trivial::error;
        break;
    case WARNING:
        boost_level = logging::trivial::warning;
        break;
    case INFO:
        boost_level = logging::trivial::info;
        break;
    case DEBUG:
        boost_level = logging::trivial::debug;
        break;
    case NOTSET:
        boost_level = logging::trivial::trace;
        break;
    default:
        boost_level = logging::trivial::info;
        break;
    }
    return boost_level;
}

void PCTSPinitLogging(int level) {
    logging::core::get()->set_filter(logging::trivial::severity >= level);
}
