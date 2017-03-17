#pragma once

#include <memory>
#include <cstdio>

template<typename... Args>
static void log_msg(const std::string& format, Args... args)
{
#ifdef DEBUG_TARGET
    int size = snprintf(nullptr, 0, format.c_str(), args...) + 1;
    std::unique_ptr<char[]> buf(new char[size]);
    snprintf(buf.get(), size, format.c_str(), args...);
    std::cout << std::string(buf.get(), buf.get() + size - 1) << "\n";
#endif
}
