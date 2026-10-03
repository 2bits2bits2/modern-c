# v1 — a cancellable context shared with workers.
add_chapter_test(context_v1 v1/context.c v1/context_test.c)
target_include_directories(context_v1 PRIVATE v1)
target_link_libraries(context_v1 PRIVATE Threads::Threads)
