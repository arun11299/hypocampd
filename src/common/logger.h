#ifndef HYPOTHALD_LOGGER_H
#define HYPOTHALD_LOGGER_H

#include <iostream>
#include <cstdio> // I' am sorry, but it is simply faster
#include <cstdlib>
#include <cstdarg>

namespace hypocampd {

    namespace logger {
        
        enum LogLevel {
            DEBUG = 0,
            INFO,
            NOTICE,
            ERROR,
            WARN,
            FATAL,
            UNSET = 6,
        };


        class ILogPolicy {
        using self = ILogPolicy;
        public:
            ILogPolicy() = default;

            void log_msg(int sev, const char* msg);

            FILE* get_file() const noexcept {
                return m_outfile;
            }

            void flushall() { fflush(m_outfile); }

        private:
            ILogPolicy(const self& other){}
            void operator= (const self& other){}

        protected:
            FILE* m_outfile = NULL;
        };

        /*
         * @class: Policy class for logging into terminal
         */
        class StdOutLogger: public ILogPolicy {
        using Base = ILogPolicy;
        public:
            StdOutLogger() {
                m_outfile = stdout;
            }
        };

        /*
         * @class: Policy class for logging into a file
         */
        class FileLogger: public ILogPolicy {
        using Base = ILogPolicy;
        public:
            FileLogger() {
                //std::string = Properties::get_log_file();
                Base::m_outfile = fopen("hypocampd.log", "w+");
                if (Base::m_outfile == NULL) {
                    std::cerr << "ERROR while creating log file. Application exiting"
                              << std::endl;
                    exit(1);
                }
            }
        };


        /*
         * @class: Main Log writing class.
         *         Logging technique is based upon the
         *         policy supplied as the template parameter
         */
        template <class LogPolicy = StdOutLogger>
        class LogRecorder {
        public:
            LogRecorder(): m_logger(), m_severity(INFO) {
            } 

            void set_log_level(int level) noexcept {
                m_severity = level;
            }

            int get_log_level() const noexcept {
                return m_severity;
            }

            void log(int severity, const char* fmt, ...);

            void log(int severity, std::string&& rec) {
                log_msg(severity, rec.c_str());
            }

            void log(int severity, const std::string& rec) {
                log_msg(severity, rec.c_str());
            }

        private:
            void log_args(int severity, const char* fmt, va_list ap);

            void log_msg(int severity, const char* msg);

        private:
            LogPolicy m_logger;
            LogLevel m_severity;   
        };


        template <class LogPolicy>
        void LogRecorder<LogPolicy>::log_msg(int severity, const char* msg) {
            m_logger.log_msg(severity, msg);
        }

        template <class LogPolicy>
        void LogRecorder<LogPolicy>::log_args(int severity, const char *fmt, va_list ap) {
            char buffer[1024 * 16];
            vsnprintf(buffer, sizeof(buffer), fmt, ap);
            log_msg(severity, buffer);
        }

        template <class LogPolicy>
        void LogRecorder<LogPolicy>::log(int severity, const char *fmt, ...) {
            va_list ap;
            ::va_start(ap, fmt);
            log_args(severity, fmt, ap);
            va_end(ap);
        }

#ifdef HC_USE_FILE_LOG
        using Log = LogRecorder<FileLogger>;
#else
        using Log = LogRecorder<StdOutLogger>;
#endif

        extern Log* get();
    
    }; // END namespace logger  

}; // END namespace hypocampd

#define LOG(severity, msg) \
    if (severity >= logger::get()->get_log_level()) { \
        logger::get()->log(severity, msg); \
    }

// __VA_ARGS__ is gcc specific macro
#define LOGF(severity, fmt, ...) \
    if (severity >= logger::get()->get_log_level()) { \
        logger::get()->log(severity, fmt, __VA_ARGS__); \
    }

#define DEBUG(msg) LOG(logger::DEBUG, msg)
#define FDEBUG(msg, ...) LOGF(logger::DEBUG, msg, __VA_ARGS__)

#define INFO(msg) LOG(logger::INFO, msg)
#define FINFO(msg, ...) LOGF(logger::INFO, msg, __VA_ARGS__)

#define NOTICE(msg) LOG(logger::NOTICE, msg)
#define FNOTICE(msg, ...) LOGF(logger::NOTICE, msg, __VA_ARGS__)

#define ERROR(msg) LOG(logger::ERROR, msg)
#define FERROR(msg, ...) LOGF(logger::ERROR, msg, __VA_ARGS__)

#define WARN(msg) LOG(logger::WARN, msg)
#define FWARN(msg, ...) LOGF(logger::WARN, msg, __VA_ARGS__)

#define FATAL(msg) LOG(logger::FATAL, msg)
#define FFATAL(msg, ...) LOGF(logger::FATAL, msg, __VA_ARGS__)


#endif // HYPOTHALD_LOGGER_H
