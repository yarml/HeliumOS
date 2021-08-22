#pragma once

namespace maths
{
    constexpr int pow(int x, int y)
    {
        if(y == 0)
            return 0;
        if(y % 2 == 0)
            return pow(x, y / 2) * pow(x, y / 2);
        return x * pow(x, y / 2) * pow(x, y / 2);
    }
}