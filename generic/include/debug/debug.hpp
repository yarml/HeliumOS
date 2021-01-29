#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <devices/serial.hpp>
#include <devices/io.hpp>

#define MSG_INFO  "INFO: "
#define MSG_DEBUG "DEBUG: "
#define MSG_ERROR "ERROR: "

#define DEBUG_TERM(c) outb(0xe9, c)
#define DEBUG_BREAK() {outw(0x8A00, 0x8A00); outw(0x8A00,0x08AE0);} do{} while(0)

extern serial com1;

void init_dbg(                  );
void debug_msg(  const char* msg);
void debug_msgln(const char* msg);


#endif /* DEBUG_HPP */