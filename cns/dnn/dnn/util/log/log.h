#pragma once

#include <string>

using std::string;

#include <dnn/base/errors.h>
#include <dnn/contrib/spdlog/spdlog.h>
#undef S

namespace NDnn {

    class TLog {
    public:
        TLog()
        {
            Log = spdlog::stdout_logger_mt("console");
            Log->set_pattern("LOG [%H:%M:%S %z] %v");
        }
        void SetColors() {
            Log->set_pattern("\x1b[32mLOG [%H:%M:%S %z] [thread %t]: %v\x1b[0m");
        }

        enum ELogLevel {INFO_LEVEL, DEBUG_LEVEL};

        void SetLogLevel(ELogLevel lev) {
            switch(lev) {
                case TLog::INFO_LEVEL:
                    spdlog::set_level(spdlog::level::info);
                    break;
                case TLog::DEBUG_LEVEL:
                    spdlog::set_level(spdlog::level::debug);
                    break;
                default:
                    throw TDnnException() << "Invalig log level\n";
            }
        }
        ELogLevel GetLogLevel() {
            if(Log->level() == spdlog::level::info) {
                return INFO_LEVEL;
            }
            return DEBUG_LEVEL;
        }
        spdlog::details::line_logger Info() {
            return Log->info();
        }

        spdlog::details::line_logger Debug() {
            return Log->debug();
        }
        
        spdlog::details::line_logger Error() {
            return Log->error();
        }
        
        static TLog& Instance();
    private:
        std::shared_ptr<spdlog::logger> Log;
    };


#define L_INFO \
    TLog::Instance().Info() << "INFO: "
#define L_DEBUG \
    TLog::Instance().Debug() << "DEBUG: "

#define L_ERROR \
    TLog::Instance().Error() << "ERROR: "

}
