#pragma once

inline bool EqualsIgnoreCase(const std::string& s1, const std::string& s2, const std::locale& loc = std::locale()) {
    const std::ctype<char>& ctype = std::use_facet<std::ctype<char>>(loc);
    const auto compareCharLower = [&](char c1, char c2) { return ctype.tolower(c1) == ctype.tolower(c2); };
    return s1.size() == s2.size() && std::equal(s1.begin(), s1.end(), s2.begin(), compareCharLower);
}

struct IgnoreCaseStringLess {
    bool operator()(const std::string& a, const std::string& b, const std::locale& loc = std::locale()) const noexcept {
        const std::ctype<char>& ctype = std::use_facet<std::ctype<char>>(loc);
        const auto ignoreCaseCharLess = [&](char c1, char c2) { return ctype.tolower(c1) < ctype.tolower(c2); };
        return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), ignoreCaseCharLess);
    }
};

template <typename T>
struct ScopeGuard {
    // Needs C++17: static_assert(std::is_invocable_v<T>, "Type must be invocable function.");

    ScopeGuard(T&& guard) : m_guard(std::move(guard)) {}

    ScopeGuard(ScopeGuard&&) = default;
    ScopeGuard& operator=(ScopeGuard&&) = default;

    ScopeGuard(ScopeGuard&) = delete;
    ScopeGuard& operator=(ScopeGuard&) = delete;

    ~ScopeGuard() { m_guard(); }

   private:
    T m_guard;
};

// Usage: auto guard = MakeScopeGuard([&] { foobar; });
template <typename T>
ScopeGuard<T> MakeScopeGuard(T&& guard) {
    return ScopeGuard<T>(std::forward<T>(guard));
}

inline std::string Fmt(const char* fmt, ...) {
    va_list vl;
    va_start(vl, fmt);
    int size = std::vsnprintf(nullptr, 0, fmt, vl);
    va_end(vl);

    if (size != -1) {
        std::unique_ptr<char[]> buffer(new char[size + 1]);

        va_start(vl, fmt);
        size = std::vsnprintf(buffer.get(), size + 1, fmt, vl);
        va_end(vl);
        if (size != -1) {
            return std::string(buffer.get(), size);
        }
    }

    throw std::runtime_error("Unexpected vsnprintf failure");
}

// The equivalent of C++17 std::size. A helper to get the dimension for an array.
template <typename T, size_t Size>
constexpr size_t ArraySize(const T (&)[Size]) noexcept {
    return Size;
}

#include "logger.h"
#include "check.h"
