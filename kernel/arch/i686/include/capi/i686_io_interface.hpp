#pragma once

#include <capi/io_interface.hpp>


namespace i686
{
    class i686_io_interface : public capi::io_interface
    {
    public:
        void write_byte(capi::ioadr, capi::byte) const override;
        void write_word(capi::ioadr, capi::word) const override;
        void write_dword(capi::ioadr, capi::dword) const override;
        void write_qword(capi::ioadr, capi::qword) const override;

        capi::byte read_byte(capi::ioadr) const override;
        capi::word read_word(capi::ioadr) const override;
        capi::dword read_dword(capi::ioadr) const override;
        capi::qword read_qword(capi::ioadr) const override;
    };
}
