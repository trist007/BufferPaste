@echo off

set CommonCompilerFlags=-MT -nologo -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -DBUFFERPASTE_INTERNAL=1 -DBUFFERPASTE_SLOW=1 -DBUFFERPASTE_WIN32=1 -FC -Z7
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib

REM TODO - can we just build both with one exe?

IF NOT EXIST ..\..\build mkdir ..\..\build
pushd ..\..\build

rc ..\bufferpaste\code\resource.rc

REM 32-bit build
REM cl %CommonCompilerFlags% ..\handmade\code\win32_handmade.cpp /link -subsystem:windows,5.1 %CommonLinkerFlags%

REM 64-bit build
REM del *.pdb > NUL 2> NUL
cl %CommonCompilerFlags% ..\bufferpaste\code\win32_bufferpaste.cpp /link %CommonLinkerFlags% ..\bufferpaste\code\resource.res
popd
