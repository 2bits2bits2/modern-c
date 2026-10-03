# v1 — dependency injection via FILE *.
add_chapter_test(di_v1 v1/greet.c v1/greet_test.c)
target_include_directories(di_v1 PRIVATE v1)

# v2 — dependency injection via our own writer abstraction.
add_chapter_test(di_v2 v2/greet.c v2/writer.c v2/buffer.c v2/greet_test.c)
target_include_directories(di_v2 PRIVATE v2)
