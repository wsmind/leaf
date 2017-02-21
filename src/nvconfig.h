#ifndef NV_CONFIG
#define NV_CONFIG

#define HAVE_UNISTD_H
#define HAVE_STDARG_H
#define HAVE_SIGNAL_H
#define HAVE_EXECINFO_H
#define HAVE_MALLOC_H

#define HAVE_OPENMP
#define HAVE_DISPATCH_H

#define HAVE_STBIMAGE
//#define HAVE_PNG
//#define HAVE_JPEG
//#define HAVE_TIFF
//#define HAVE_OPENEXR
//#define HAVE_FREEIMAGE

#define HAVE_MAYA

#define HAVE_CUDA

// missing class used in CudaUtils.cpp
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <Windows.h>
#undef _WIN32_WINNT
class Library
{
    public:
        Library(const char *name)
        {
            this->module = LoadLibrary(name);
        }

        ~Library()
        {
            FreeLibrary(this->module);
        }

        bool isValid()
        {
            return this->module != nullptr;
        }

        void *bindSymbol(const char *name)
        {
            return GetProcAddress(this->module, name);
        }

    private:
        HMODULE module;
};

#endif // NV_CONFIG
