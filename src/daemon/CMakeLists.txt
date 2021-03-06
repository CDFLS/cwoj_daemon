set(Boost_USE_STATIC_LIBS ON)
find_package(Threads)
find_package(Boost COMPONENTS system filesystem REQUIRED)
find_package(Yaml-cpp)
find_package(seccomp)

aux_source_directory(. DAEMON_SOURCE_FILES)
aux_source_directory(./sandboxing/ DAEMON_SOURCE_FILES)
aux_source_directory(./log/ DAEMON_SOURCE_FILES)
aux_source_directory(./config/ DAEMON_SOURCE_FILES)
aux_source_directory(./util/ DAEMON_SOURCE_FILES)
add_executable(cwoj_daemon_daemon ${DAEMON_SOURCE_FILES})

target_link_libraries(cwoj_daemon_daemon mysqlclient)
target_link_libraries(cwoj_daemon_daemon microhttpd)
target_link_libraries(cwoj_daemon_daemon ${Boost_LIBRARIES})
target_link_libraries(cwoj_daemon_daemon ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(cwoj_daemon_daemon yaml-cpp)
target_link_libraries(cwoj_daemon_daemon seccomp)

install(PROGRAMS ${CMAKE_SOURCE_DIR}/bin/cwoj_daemon_daemon DESTINATION bin)

include("../../ConfigFile.cmake")
CopyConfigFileIfNotExist(
        "${CMAKE_SOURCE_DIR}/cwojconfig.ini"
        "/etc/cwojconfig.ini"
        "/etc/"
)
CopyConfigFileIfNotExist(
        "${CMAKE_SOURCE_DIR}/cwojdaemon.service"
        "/lib/systemd/system/cwojdaemon.service"
        "/lib/systemd/system/"
)
