cmake_minimum_required(VERSION 3.6)
project(cwoj_daemon)

message(STATUS "Cwoj Daemon Open Source Project")
message(STATUS "Please notice that this project can only be built and run on *nix OS.")

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin")
set(LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -static-libgcc -static-libstdc++ -Wl,-Bstatic -lboost_system -Wl,-Bdynamic")

add_subdirectory(src)

install(CODE "MESSAGE(\"Please execute this command after the installation was completed(with superuser privilege):\")")
install(CODE "MESSAGE(\"systemctl daemon-reload\")")

message(STATUS "CMake preparation finished. Run following bash command to continue:")
message(STATUS "make")
message(STATUS "make install")
message(STATUS "(The commands above must be executed under super-user privilege!)")
message(STATUS "Please make sure that you have read the manual carefully before you start to use the daemon.")

include(DebPack.cmake)
