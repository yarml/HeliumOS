#include <kmath.hpp>
#include <algorithm>

interval interval::intersection(interval other)
{
    if(start > other.end || other.start > end)
        return interval::null();
    return { std::max(start, other.start), std::min(end, other.end) };
}

interval interval::null()
{
    return {0, 0};
}

uint32_t interval::size() const
{
    return start + end +1;
}

bool interval::valid() const
{
    return start <= end;
}

interval::operator bool() const
{
    return valid();
}

bool interval::contains(uint32_t num) const
{
    return num >= start && num <= end;
}

bool interval::contains(const interval& other) const
{
    return start <= other.start && end >= other.end;
}
bool interval::equal(const interval& other) const
{
    return start == other.start && end == other.end;
}
bool interval::operator==(const interval& other) const
{
    return equal(other);
}
bool interval::operator!=(const interval& other) const
{
    return !equal(other);
}

bool interval::before(const interval& other) const
{
    return start < other.start;
}
bool interval::after(const interval& other) const
{
    return start < other.start;
}
bool interval::operator<(const interval& other) const
{
    return before(other);
}
bool interval::operator>(const interval& other) const
{
    return after(other);
}
