setlocal
c:\Mingw\bin\mingw32-make %* 2>err.log
head err.log
