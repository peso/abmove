setlocal
set archname=abmove-0.1
rmdir /s/q %archname%
del /q %archname%.tar.gz

hg archive -t files %archname%
del %archname%\.hg*
del %archname%\*.bat
rem hg archive -t tgz -p %archname% %archname%.tar.gz
tar cf %archname%.tar %archname%
gzip %archname%.tar
