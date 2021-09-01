#pragma once

#include <utils/types.hpp>

constexpr utils::ull operator""_Kib(utils::ull v)
{
    return v * 1024;
}
constexpr utils::ull operator""_Mib(utils::ull v)
{
    return v * 1024 * 1024;
}
constexpr utils::ull operator""_Gib(utils::ull v)
{
    return v * 1024 * 1024 * 1024;
}
