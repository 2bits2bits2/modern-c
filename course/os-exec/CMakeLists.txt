# v1 — running a program and capturing its output.
add_chapter_test(os_exec_v1 v1/exec_test.c exec.c)
target_include_directories(os_exec_v1 PRIVATE v1 ${CMAKE_CURRENT_SOURCE_DIR})

# v2 — injectable command running, so logic is testable without a process.
add_chapter_test(os_exec_v2 v2/user_test.c v2/user.c exec.c)
target_include_directories(os_exec_v2 PRIVATE v2
                                               ${CMAKE_CURRENT_SOURCE_DIR})
