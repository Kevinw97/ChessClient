#ifdef CHESS_SERVER_BUILD
#pragma once
#include <iostream>
#include <sstream>

#define PRINT_MSG(msg)                        \
    do {                                      \
        std::ostringstream _oss;              \
        _oss << msg;                          \
        std::cout << _oss.str() << std::endl; \
    } while (0)

#define THROW_RUNTIME_ERROR(msg)              \
    do {                                      \
        std::ostringstream _oss;              \
        _oss << msg;                          \
        throw std::runtime_error(_oss.str()); \
    } while (0)

#endif