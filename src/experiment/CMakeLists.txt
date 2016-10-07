find_package(Yaml-cpp)

aux_source_directory(. EXPERIMENT_SOURCE_FILE)

add_executable(cwoj_experiment ${EXPERIMENT_SOURCE_FILE})

target_link_libraries(cwoj_experiment yaml-cpp)
