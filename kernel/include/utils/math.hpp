#pragma once

namespace math
{
    template<typename T>
    const T& min(const T& t1, const T& t2)
    {
        return t1 <= t2 ? t1 : t2;
    }
    template<typename T>
    const T& max(const T& t1, const T& t2)
    {
        return t1 >= t2 ? t1 : t2;
    }
}
