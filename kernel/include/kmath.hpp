#ifndef KMATH_HPP
#define KMATH_HPP

#include <debug/debug.hpp>
#include <stdint.h>

struct interval
{
public:
    static interval null();
public:
    uint32_t start;
    uint32_t end;
public:
    interval intersection(interval other);
public:
    uint32_t size()                      const;
    bool valid()                         const;
    bool contains(uint32_t num)          const;
    bool contains(const interval& other) const;
    bool before(  const interval& other) const;
    bool after(   const interval& other) const;
    bool equal(   const interval& other) const;
public:
    operator bool()                        const;
    bool operator<( const interval& other) const;
    bool operator>( const interval& other) const;
    bool operator==(const interval& other) const;
    bool operator!=(const interval& other) const;
public:
    friend debug& operator<<(debug&, const interval&);
};

#endif /* KMATH_HPP */