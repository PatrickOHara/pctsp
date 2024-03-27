
#include "pctsp/logger.hh"
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/support/date_time.hpp>  

namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;

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
    logging::add_common_attributes();
    logging::core::get()->set_filter(logging::trivial::severity >= level);
    keywords::format =
    (
        expr::stream
            << expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S")
            << ": <" << logging::trivial::severity
            << "> " << expr::smessage
    );
}
