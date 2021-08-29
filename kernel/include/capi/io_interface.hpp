#pragma once

#include <capi/types.hpp>

namespace capi
{
    class io_interface
    {
    public:
        // virtual return_t f(args); // needed flags
        virtual void write_byte(ioadr, byte) const = 0; // IO_BYTE_W
        virtual void write_word(ioadr, word) const = 0; // IO_WORD_W
        virtual void write_dword(ioadr, dword) const = 0; // IO_DWORD_W
        virtual void write_qword(ioadr, qword) const = 0; // IO_QWORD_W

        virtual byte read_byte(ioadr) const = 0; // IO_BYTE_R
        virtual word read_word(ioadr) const = 0; // IO_WORD_R
        virtual dword read_dword(ioadr) const = 0; // IO_DWORD_R
        virtual qword read_qword(ioadr) const = 0; // IO_QWORD_R
    };
}
