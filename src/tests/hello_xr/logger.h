#pragma once

namespace Log {
enum class Level { Verbose, Info, Warning, Error };

void SetLevel(Level minLevel);
void Write(Level level, const std::string& msg);
}  // namespace Log
