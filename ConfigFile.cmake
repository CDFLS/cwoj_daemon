function(CopyConfigFileIfNotExists src cond dest)
    if (NOT EXISTS ${cond})
        install(FILES ${src} DESTINATION ${dest})
    else ()
        install(CODE "message(STATUS \"${cond} is exists, template copying will be passed in the installation process.\")")
    endif ()
endfunction()
