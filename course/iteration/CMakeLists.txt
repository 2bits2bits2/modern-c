# v1 — repeat a string into a buffer.
add_chapter_test(iteration_v1 v1/repeat.c v1/repeat_test.c)
target_include_directories(iteration_v1 PRIVATE v1)

# v2 — a benchmark program.
add_chapter_program(iteration_v2_bench v1/repeat.c v2/bench.c)
target_include_directories(iteration_v2_bench PRIVATE v1)
