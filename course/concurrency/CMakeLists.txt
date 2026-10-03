# v1 — check websites sequentially and concurrently.
add_chapter_test(concurrency_v1 v1/website.c v1/website_test.c)
target_include_directories(concurrency_v1 PRIVATE v1)
target_link_libraries(concurrency_v1 PRIVATE Threads::Threads)

# v2 — a benchmark that shows the difference.
add_chapter_program(concurrency_v2_bench v1/website.c v2/bench.c)
target_include_directories(concurrency_v2_bench PRIVATE v1)
target_link_libraries(concurrency_v2_bench PRIVATE Threads::Threads)
