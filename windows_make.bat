 cd %~dp0
 del /F /Q build
 mkdir build
 mkdir build\img
 copy img build\img\
 cd build
 cmake ..
 pause