#pragma once

#include <capi/io_interface.hpp>


namespace i686
{
    class i686_io_interface : public capi::io_interface
    {
    public:
        void write_byte(capi::ioadr, capi::byte) override;
        void write_word(capi::ioadr, capi::word) override;
        void write_dword(capi::ioadr, capi::dword) override;
        void write_qword(capi::ioadr, capi::qword) override;

        capi::byte read_byte(capi::ioadr) override;
        capi::word read_word(capi::ioadr) override;
        capi::dword read_dword(capi::ioadr) override;
        capi::qword read_qword(capi::ioadr) override;
    };
}
