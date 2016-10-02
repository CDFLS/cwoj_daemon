cmake_minimum_required(VERSION 3.6)
project(cwoj_daemon)

message(STATUS "Cwoj Daemon Open Source Project")
message(STATUS "Please notice that this project can only be built and run on *nix OS.")

set(EXECUTABLE_OUTPUT_PATH "bin")
set(LIBRARY_OUTPUT_DIRECTORY "lib")

find_package(Threads)
find_package(Boost COMPONENTS system filesystem REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

aux_source_directory(src/daemon/ DAEMON_SOURCE_FILES)
aux_source_directory(src/runner/ RUNNER_SOURCE_FILES)

add_executable(cwoj_daemon_daemon ${DAEMON_SOURCE_FILES})
add_executable(cwoj_daemon_runner ${RUNNER_SOURCE_FILES})

target_link_libraries(cwoj_daemon_daemon mysqlclient)
target_link_libraries(cwoj_daemon_daemon microhttpd)
target_link_libraries(cwoj_daemon_daemon ${Boost_LIBRARIES})
target_link_libraries(cwoj_daemon_daemon ${CMAKE_THREAD_LIBS_INIT})

install(PROGRAMS bin/cwoj_daemon_daemon DESTINATION bin)
install(PROGRAMS bin/cwoj_daemon_runner DESTINATION bin)
install(FILES cwojconfig.ini DESTINATION /etc/)
install(FILES cwojdaemon.service DESTINATION /etc/systemd/system/)
install(CODE "MESSAGE(\"Please execute this command after the installation was completed:\")")
install(CODE "MESSAGE(\"systemctl daemon-reload\")")

message(STATUS "CMake preparation finished. Run following bash command to continue:")
message(STATUS "make")
message(STATUS "make install")
message(STATUS "(The commands above must be executed under super-user privilege!)")
message(STATUS "Please make sure that you have read the manual carefully before you start to use the daemon.")
