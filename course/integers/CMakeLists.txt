# v1 — add two integers.
add_chapter_test(integers_v1 v1/adder.c v1/adder_test.c)
target_include_directories(integers_v1 PRIVATE v1)

# v2 — checked addition that reports overflow.
add_chapter_test(integers_v2 v2/adder.c v2/adder_test.c)
target_include_directories(integers_v2 PRIVATE v2)
