set(SOURCES "filelist.c" "group.c" "main.c")

add_executable("simdb-fdupes" ${SOURCES})
set_property(TARGET "simdb-fdupes" PROPERTY LINK_FLAGS "-Wl,--as-needed")
target_link_libraries("simdb-fdupes" "magic")
target_link_libraries("simdb-fdupes" "simdb")
install(TARGETS "simdb-fdupes" RUNTIME DESTINATION "bin")
