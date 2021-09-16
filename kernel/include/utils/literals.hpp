#pragma once

#include <utils/types.hpp>

constexpr utils::large_int operator""_Kib(utils::large_int v)
{
    return v * 1024;
}
constexpr utils::large_int operator""_Mib(utils::large_int v)
{
    return v * 1024 * 1024;
}
constexpr utils::large_int operator""_Gib(utils::large_int v)
{
    return v * 1024 * 1024 * 1024;
}
