#if (defined(__GNUC__) || defined(__GNUG__) || defined(__clang__)) && !defined(__INTEL_COMPILER)
extern "C" void __cxa_pure_virtual()
{
    // TODO: maybe display error?
    // this function can be called from 
    // architecture specific section,
    // so the error handling should be
    // compatible with those
}
#endif
