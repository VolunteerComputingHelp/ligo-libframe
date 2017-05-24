from distutils.core import setup, Extension
import numpy, os

# FrameL is not consistent in its installation across platforms since
# PowerPC's architecture string contains a space ("Power Macintosh").
# The following works for Linux and Mac OS X
FrameLlibdir = '../' + os.uname()[0]
if FrameLlibdir=='../Linux':
    FrameLlibdir += '-' + os.uname()[4]
FrameLlibdir = '../Linux-x86_64-SL5'

Fr = Extension('Fr',
        extra_compile_args = ['-w'],
        include_dirs = ['../src',numpy.get_include()],
        libraries = ['Frame'],
        library_dirs = [FrameLlibdir],
        sources = ['Fr.c'],
        )

setup (name = 'Fr',
       version = '0.3',
       description = 'FrameL-based frame reading interface for Python',
       ext_modules = [Fr])
       
