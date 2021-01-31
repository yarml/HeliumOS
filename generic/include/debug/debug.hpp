#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <dev/serial.hpp>
#include <dev/io.hpp>

#define DEBUG_TERM(c) outb(0xe9, c)
#define DEBUG_BREAK() {outw(0x8A00, 0x8A00); outw(0x8A00,0x08AE0);} do{} while(0)

class debug
{
private:
    serial* mp_serial;
public:
    debug(serial& serial);
public:
    void print(char c         );
    void print(const char* msg);
    void print(uint32_t    n  );
    void print(void*       ptr);

    debug& operator<<(char c         );
    debug& operator<<(const char* msg);
    debug& operator<<(uint32_t    n  );
    debug& operator<<(void*       ptr);
};

extern debug dbg;

#endif /* DEBUG_HPP */