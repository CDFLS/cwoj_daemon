find_package(Threads)
find_package(Boost COMPONENTS system filesystem REQUIRED)

aux_source_directory(. DAEMON_SOURCE_FILES)
add_executable(cwoj_daemon_daemon ${DAEMON_SOURCE_FILES})

target_link_libraries(cwoj_daemon_daemon mysqlclient)
target_link_libraries(cwoj_daemon_daemon microhttpd)
target_link_libraries(cwoj_daemon_daemon ${Boost_LIBRARIES})
target_link_libraries(cwoj_daemon_daemon ${CMAKE_THREAD_LIBS_INIT})

install(PROGRAMS ${CMAKE_SOURCE_DIR}/bin/cwoj_daemon_daemon DESTINATION bin)

if (NOT EXISTS "/etc/cwojconfig.ini")
    install(FILES ${CMAKE_SOURCE_DIR}/cwojconfig.ini DESTINATION /etc/)
else ()
    message(STATUS "File /etc/cwojconfig.ini is exists, template copying will be passed in the installation process.")
endif ()

if (NOT EXISTS "/etc/systemd/system/cwojdaemon.service")
    install(FILES ${CMAKE_SOURCE_DIR}/cwojdaemon.service DESTINATION /etc/systemd/system/)
else ()
    message(STATUS "File /etc/systemd/system/cwojdaemon.service is exists, template copying will be passed in the installation process.")
endif ()
