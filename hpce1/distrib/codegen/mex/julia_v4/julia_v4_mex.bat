@echo off
set MATLAB=E:\PROGRA~2\MATLAB\R2013a
set MATLAB_ARCH=win64
set MATLAB_BIN="E:\Program Files\MATLAB\R2013a\bin"
set ENTRYPOINT=mexFunction
set OUTDIR=.\
set LIB_NAME=julia_v4_mex
set MEX_NAME=julia_v4_mex
set MEX_EXT=.mexw64
call mexopts.bat
echo # Make settings for julia_v4 > julia_v4_mex.mki
echo COMPILER=%COMPILER%>> julia_v4_mex.mki
echo COMPFLAGS=%COMPFLAGS%>> julia_v4_mex.mki
echo OPTIMFLAGS=%OPTIMFLAGS%>> julia_v4_mex.mki
echo DEBUGFLAGS=%DEBUGFLAGS%>> julia_v4_mex.mki
echo LINKER=%LINKER%>> julia_v4_mex.mki
echo LINKFLAGS=%LINKFLAGS%>> julia_v4_mex.mki
echo LINKOPTIMFLAGS=%LINKOPTIMFLAGS%>> julia_v4_mex.mki
echo LINKDEBUGFLAGS=%LINKDEBUGFLAGS%>> julia_v4_mex.mki
echo MATLAB_ARCH=%MATLAB_ARCH%>> julia_v4_mex.mki
echo BORLAND=%BORLAND%>> julia_v4_mex.mki
echo OMPFLAGS= >> julia_v4_mex.mki
echo OMPLINKFLAGS= >> julia_v4_mex.mki
echo EMC_COMPILER=msvc110>> julia_v4_mex.mki
echo EMC_CONFIG=optim>> julia_v4_mex.mki
"E:\Program Files\MATLAB\R2013a\bin\win64\gmake" -B -f julia_v4_mex.mk
