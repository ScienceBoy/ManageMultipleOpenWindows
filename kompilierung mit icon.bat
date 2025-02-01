taskkill /IM managemultipleopenwindows.exe /F
windres resource.rc -o resource.o
g++ -std=c++17 ManageMultipleOpenWindows.cpp resource.o -o ManageMultipleOpenWindows -lgdi32 -luser32 -lcomctl32 -lpsapi -ldwmapi -lpthread -static-libgcc -static-libstdc++ -static -O3 -s -DNDEBUG -mwindows
start "" "ManageMultipleOpenWindows.exe"