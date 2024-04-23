#ifndef INCLUDE_GPHOTO2PP_LOG_HPP_
#define INCLUDE_GPHOTO2PP_LOG_HPP_

// from here http://stackoverflow.com/questions/5028302/small-logger-class

#include <stdio.h>
#include <sstream>
#include <string>
#include <iostream>
#include <mutex>

inline std::string NowTime();

enum TLogLevel {
    logEMERGENCY,
    logALERT,
    logCRITICAL,
    logERROR,
    logWARN,
    logWARN1,
    logWARN2,
    logWARN3,
    logINFO,
    logDEBUG,
    logDEBUG1,
    logDEBUG2,
    logDEBUG3
};

template <typename T>
class Log {
 public:
    Log();
    virtual ~Log();
    std::ostringstream& Get(TLogLevel level = logINFO);
    static TLogLevel& ReportingLevel();
    static std::string ToString(TLogLevel level);
    static TLogLevel FromString(const std::string& level);
    static std::string showFunction(TLogLevel level, const std::string& fname, uint line);

 protected:
    std::ostringstream os;

 private:
    static std::string rPadTo(const std::string &str, const size_t num, const char paddingChar = ' ');
    Log(const Log&);
    Log& operator =(const Log&);
};

template <typename T>
Log<T>::Log() {
}

template <typename T>
std::ostringstream& Log<T>::Get(TLogLevel level) {
    os << "- " << NowTime();
    os << " " << ToString(level) << ": ";
    os << std::string(level > logWARN && level <= logWARN3 ? level - logWARN : 0, '\t');
    return os;
}

template <typename T>
Log<T>::~Log() {
    if (os.str().size() > 0) {
        os << std::endl;
        T::Output(os.str());
    }
}

template <typename T>
TLogLevel& Log<T>::ReportingLevel() {
    static TLogLevel reportingLevel = logERROR;
    return reportingLevel;
}

template <typename T>
std::string Log<T>::rPadTo(const std::string &str, const size_t num, const char paddingChar) {
    std::string out = str;
    if (num > out.size()) out.insert(str.size(), num - str.size(), paddingChar);
    return out;
}

template <typename T>
std::string Log<T>::showFunction(TLogLevel level, const std::string &fname, uint line) {
    return  (ReportingLevel() >= logDEBUG)? "["+ fname+ "]["+std::to_string(line)+"] " : "";
}

template <typename T>
std::string Log<T>::ToString(TLogLevel level) {
    static const char* const buffer[] = {
        "EMERGENCY",
        "ALERT",
        "CRITICAL",
        "ERROR",
        "WARN",
        "WARN1",
        "WARN2",
        "WARN3",
        "INFO",
        "DEBUG",
        "DEBUG1",
        "DEBUG2",
        "DEBUG3"
    };

    return rPadTo(buffer[level], 9);
}

template <typename T>
TLogLevel Log<T>::FromString(const std::string& level)
{
    TLogLevel logLevel = logINFO;

    switch (level) {
        case "DEBUG3"   : logLevel = logDEBUG3;     break;
        case "DEBUG2"   : logLevel = logDEBUG2;     break;
        case "DEBUG1"   : logLevel = logDEBUG1;     break;
        case "DEBUG"    : logLevel = logDEBUG;      break;
        case "WARN3"    : logLevel = logWARN3;      break;
        case "WARN2"    : logLevel = logWARN2;      break;
        case "WARN1"    : logLevel = logWARN1;      break;
        case "WARN"     : logLevel = logWARN;       break;
        case "ERROR"    : logLevel = logERROR;      break;
        case "CRITICAL" : logLevel = logCRITICAL;   break;
        case "ALERT"    : logLevel = logALERT;      break;
        case "EMERGENCY": logLevel = logEMERGENCY;  break;
        case "INFO"     :                           break;
        default:
            Log<T>().Get(logWARN) << "Unknown logging level '" << level << "'. Using INFO level as default.";
            break;
    }

    return logLevel;
}

#define LOG_STDERR 0x0001
#define LOG_STDOUT 0x0002

class Output2FILE {
 private:
    /// @brief  controls multi thread access
    static std::recursive_mutex &mutex();
 public:
    static uint16_t& WriteToStdOut();
    static std::ostream * &Stream();
    static void Output(const std::string& msg);
};

inline std::recursive_mutex & Output2FILE::mutex() {
        static std::recursive_mutex m_mutex;
        return m_mutex;
    }

inline uint16_t& Output2FILE::WriteToStdOut() {
    static uint16_t stdout_flags = 0;
    return stdout_flags;
}

inline std::ostream *&Output2FILE::Stream() {
    static std::ostream *stream = &std::cerr;
    return stream;
}

inline void Output2FILE::Output(const std::string& msg) {
    std::lock_guard<std::recursive_mutex> lock{mutex()};

    auto stream = Stream();
    if (stream) {
        *stream << msg;
        stream->flush();
    }

    if (WriteToStdOut() & LOG_STDERR) {
        std::cerr << msg;
    }

    if (WriteToStdOut() & LOG_STDOUT) {
        std::cout << msg;
    }
}

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#   if defined (BUILDING_FILELOG_DLL)
#       define FILELOG_DECLSPEC   __declspec (dllexport)
#   elif defined (USING_FILELOG_DLL)
#       define FILELOG_DECLSPEC   __declspec (dllimport)
#   else
#       define FILELOG_DECLSPEC
#   endif  // BUILDING_DBSIMPLE_DLL
#else
#   define FILELOG_DECLSPEC
#endif  // _WIN32

class FILELOG_DECLSPEC FILELog : public Log<Output2FILE> {};
// typedef Log<Output2FILE> FILELog;

#ifndef FILELOG_MAX_LEVEL
    #define FILELOG_MAX_LEVEL logDEBUG3
#endif

#define FILE_LOG(level) \
    if (level > FILELOG_MAX_LEVEL) ;\
    else if (level > FILELog::ReportingLevel() || !Output2FILE::Stream()) ; \
    else FILELog().Get(level) << FILELog().showFunction(level, __FUNCTION__, __LINE__)

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)

#include <windows.h>

inline std::string NowTime() {
    const int MAX_LEN = 200;
    char buffer[MAX_LEN];
    if (GetTimeFormatA(LOCALE_USER_DEFAULT, 0, 0,
            "HH':'mm':'ss", buffer, MAX_LEN) == 0)
        return "Error in NowTime()";

    char result[100] = {0};
    static DWORD first = GetTickCount();
    std::snprintf(result, sizeof(result)-1, "%s.%03ld", buffer, (int64)(GetTickCount() - first) % 1000);
    return result;
}

#else

#include <sys/time.h>

inline std::string NowTime() {
    char buffer[11];
    time_t t;
    time(&t);
    tm r = {0};
    strftime(buffer, sizeof(buffer)-1, "%X", localtime_r(&t, &r));
    struct timeval tv;
    gettimeofday(&tv, 0);
    char result[100] = {0};
    std::snprintf(result, sizeof(result)-1, "%s.%03ld", buffer, (int64_t)tv.tv_usec / 1000);
    return result;
}

#endif //WIN32

#include <stdarg.h>
#include <vector>
inline std::string StringFormat(const char *format, ...) {
    std::string result;
    va_list args, args_copy;

    va_start(args, format);
    va_copy(args_copy, args);

    int len = vsnprintf(nullptr, 0, format, args);
    if (len < 0) {
        va_end(args_copy);
        va_end(args);
        throw std::runtime_error("vsnprintf error");
    } else if (len > 0) {
        std::vector<char> buffer(len+1);
        vsnprintf(&buffer[0], buffer.size(), format, args_copy);
        result = std::string(&buffer[0], len);
    }

    va_end(args_copy);
    va_end(args);
    return result;
}

#endif  // INCLUDE_GPHOTO2PP_LOG_HPP_
