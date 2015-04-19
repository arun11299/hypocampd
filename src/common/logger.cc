#include "common/logger.h"
#include <mutex>
#include <ctime>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

namespace hypocampd {
    namespace logger {

        static std::mutex log_mutex;
        static Log* log_instance = NULL;

        Log* get() {
            // Slightly modified version of DCLP
            // for better performance
            if (log_instance) {
                return log_instance;
            }

            std::unique_lock<std::mutex> l(log_mutex, std::defer_lock);
            l.lock(); 
            if (log_instance) {
                l.unlock();
                return log_instance;
            }
            log_instance = new Log();
            l.unlock();
            return log_instance;
        }

        static const char* sev_2_str[] =  {
                                   "DEBUG",
                                   "INFO",
                                   "NOTICE",
                                   "ERROR",
                                   "WARN",
                                   "FATAL",
                                };

        static inline int gettid() {
            return (int)syscall(__NR_gettid);
        }

        // Get current date/time, format is YYYY-MM-DD.HH:mm:ss
        const std::string currentDateTime() {
            time_t     now = time(0);
            struct tm  tstruct;
            char       buf[80];
            tstruct = *localtime(&now);
            // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
            // for more information about date/time format
            strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

            return buf;
        }

        void ILogPolicy::log_msg(int severity, const char* msg) {

            std::lock_guard<std::mutex> _(log_mutex);
            fprintf(m_outfile, "%s [%d/%d] %s : %s\n", currentDateTime().c_str(), 
                                                       (int)getpid(), gettid(),
                                                       sev_2_str[severity],
                                                       msg);
            flushall();
        }

    }; // END namespace logger
}; // END namespace hypocampd
