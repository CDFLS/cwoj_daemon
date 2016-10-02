cmake_minimum_required(VERSION 3.6)
project(cwoj_daemon)

find_package(Threads)
find_package(Boost COMPONENTS system filesystem REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

aux_source_directory(src/daemon/ DAEMON_SOURCE_FILES)
aux_source_directory(src/runner/ RUNNER_SOURCE_FILES)

add_executable(cwoj_daemon_daemon ${DAEMON_SOURCE_FILES})
add_executable(cwoj_daemon_runner ${RUNNER_SOURCE_FILES})

target_link_libraries(cwoj_daemon_daemon mysqlclient)
target_link_libraries(cwoj_daemon_runner mysqlclient)
target_link_libraries(cwoj_daemon_daemon microhttpd)
target_link_libraries(cwoj_daemon_runner microhttpd)
target_link_libraries(cwoj_daemon_daemon ${Boost_LIBRARIES})
target_link_libraries(cwoj_daemon_runner ${Boost_LIBRARIES})
target_link_libraries(cwoj_daemon_daemon ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(cwoj_daemon_runner ${CMAKE_THREAD_LIBS_INIT})
