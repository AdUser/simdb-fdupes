add_executable("test-filelist" "filelist.c" "../src/filelist.c")
add_test("test/filelist" "test-filelist")

add_executable("test-group" "group.c" "../src/group.c")
add_test("test/group" "test-group")
