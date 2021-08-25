#include <support/compilers.hpp>

#if GCC_COMPATIBLE
extern "C" void __cxa_pure_virtual()
{
    // TODO: maybe display error?
    // this function can be called from 
    // architecture specific section,
    // so the error handling should be
    // compatible with those
}
#endif
