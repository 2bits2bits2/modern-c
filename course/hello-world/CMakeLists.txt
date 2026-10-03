# v1 — return a constant greeting.
add_chapter_test(hello_v1 v1/hello.c v1/hello_test.c)
target_include_directories(hello_v1 PRIVATE v1)

add_chapter_program(hello_v1_program v1/hello.c v1/main.c)
target_include_directories(hello_v1_program PRIVATE v1)

# v2 — greeting a name, written into a caller-provided buffer.
add_chapter_test(hello_v2 v2/hello.c v2/hello_test.c)
target_include_directories(hello_v2 PRIVATE v2)

add_chapter_program(hello_v2_program v2/hello.c v2/main.c)
target_include_directories(hello_v2_program PRIVATE v2)

# v3 — an empty name defaults to "World".
add_chapter_test(hello_v3 v3/hello.c v3/hello_test.c)
target_include_directories(hello_v3 PRIVATE v3)

# v4 — greetings in several languages, with a switch-free helper.
add_chapter_test(hello_v4 v4/hello.c v4/hello_test.c)
target_include_directories(hello_v4 PRIVATE v4)

add_chapter_program(hello_v4_program v4/hello.c v4/main.c)
target_include_directories(hello_v4_program PRIVATE v4)
