aux_source_directory(. RUNNER_SOURCE_FILES)

add_executable(cwoj_daemon_runner ${RUNNER_SOURCE_FILES})

install(PROGRAMS ${CMAKE_SOURCE_DIR}/bin/cwoj_daemon_runner DESTINATION bin)

set (CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS} -fpermissive")
