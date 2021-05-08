#include <sys/cmds.hpp>
#include <debug/debug.hpp>
#include <dev/framebuffer.hpp>
#include <dev/terminal.hpp>
#include <kutils.hpp>
#include <memory/memory.hpp>

namespace sys
{
    static char cmd[COMMAND_BUFFER];
    void execute(const char* cmd_line)
    {
        dbg << "Excecuting: " << cmd_line << '\n';
        uint32_t space_pos = kutils::char_pos(cmd_line, ' ');
        
        if(space_pos == (uint32_t) -1)
            kutils::string_copy(cmd, cmd_line);
        else
        {
            kutils::mem_copy(cmd, cmd_line, space_pos);
            cmd[space_pos] = 0;
        }
        if(kutils::string_compare(cmd, "clear") == 0)
            terminal::reset(false);
        else if(kutils::string_compare(cmd, "memsize") == 0)
        {
            uint32_t mem_size = memory::size();
            uint32_t mem_size_size = kutils::uint32_length(mem_size);
            char mem_size_str[mem_size_size + 1];
            kutils::uint32_to_string_dec(mem_size, mem_size_str);
            mem_size_str[mem_size_size] = 0;
            terminal::write("Memory Size: ", false);
            terminal::write(mem_size_str, false);
            terminal::write(" bytes\r", false);
        }
        else if(kutils::string_compare(cmd, "kallocvp") == 0)
        {
            uint32_t vp = memory::kallocvp();
            if(vp == INVALID_PAGE)
                terminal::write("Not enough memory\r", false, RED);
            else
            {
                uint32_t vp_size = kutils::uint32_length(vp);
                char vp_str[vp_size];
                kutils::uint32_to_string_dec(vp, vp_str);
                vp_str[vp_size] = 0;
                terminal::write("Allocated new page at: ", false);
                terminal::write(vp_str, false);
                terminal::write('\r', false);
            }
        }
        else
        {
            terminal::write("Unknown command: ", false, RED);
            terminal::write(cmd, false, framebuffer_color::RED);
            terminal::write('\r', false);
        }
        cmd[0] = 0;
    }
}
