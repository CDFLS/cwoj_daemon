find_package(Threads)
find_package(Boost COMPONENTS system filesystem REQUIRED)

aux_source_directory(. DAEMON_SOURCE_FILES)
add_executable(cwoj_daemon_daemon ${DAEMON_SOURCE_FILES})

target_link_libraries(cwoj_daemon_daemon mysqlclient)
target_link_libraries(cwoj_daemon_daemon microhttpd)
target_link_libraries(cwoj_daemon_daemon ${Boost_LIBRARIES})
target_link_libraries(cwoj_daemon_daemon ${CMAKE_THREAD_LIBS_INIT})

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
