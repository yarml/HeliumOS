// This file checks wether the used compiler is supported

#include <support/compilers.hpp>

#if INTEL_COMPILER
#error Intel compiler not supported yet!
#elif MSVC_COMPILER
#error Microsoft compiler not supported yet
#endif



// TODO: Support the intel and microsoft compiler
