#include "pch.h"
#include "common.h"
#include "options.h"
#include "logger.h"

#include <sstream>

namespace {
Log::Level g_minSeverity{Log::Level::Info};
std::mutex g_logLock;
}  // namespace

namespace Log {
void SetLevel(Level minSeverity) { g_minSeverity = minSeverity; }

void Write(Level severity, const std::string& msg) {
    if (severity < g_minSeverity) {
        return;
    }

    const auto now = std::chrono::system_clock::now();
    const time_t now_time = std::chrono::system_clock::to_time_t(now);
    tm now_tm;
#ifdef _MSC_VER
    localtime_s(&now_tm, &now_time);
#else
    localtime_r(&now_time, &now_tm);
#endif
    // time_t only has second precision. Use the rounding error to get sub-second precision.
    const auto secondRemainder = now - std::chrono::system_clock::from_time_t(now_time);
    const int64_t milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(secondRemainder).count();

    static std::map<Level, const char*> severityName = {
        {Level::Verbose, "Verbose"}, {Level::Info, "Info   "}, {Level::Warning, "Warning"}, {Level::Error, "Error  "}};

    std::ostringstream out;
    out.fill('0');
    out << "[" << std::setw(2) << now_tm.tm_hour << ":" << std::setw(2) << now_tm.tm_min << ":" << std::setw(2) << now_tm.tm_sec
        << "." << std::setw(3) << milliseconds << "]"
        << "[" << severityName[severity] << "] " << msg << std::endl;

    std::lock_guard<std::mutex> lock(g_logLock);  // Ensure output is serialized
    ((severity == Level::Error) ? std::clog : std::cout) << out.str();
#if defined(_WIN32)
    OutputDebugStringA(out.str().c_str());
#endif
}
}  // namespace Log
