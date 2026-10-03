# v1 — a mutex-protected counter.
add_chapter_test(sync_v1 v1/counter.c v1/counter_test.c)
target_include_directories(sync_v1 PRIVATE v1)
target_link_libraries(sync_v1 PRIVATE Threads::Threads)

# v2 — a WaitGroup built from a mutex and a condition variable.
add_chapter_test(sync_v2 v2/waitgroup.c v2/waitgroup_test.c)
target_include_directories(sync_v2 PRIVATE v2)
target_link_libraries(sync_v2 PRIVATE Threads::Threads)

# v3 — the same counter with atomics, and no lock.
add_chapter_test(sync_v3 v3/counter.c v3/counter_test.c)
target_include_directories(sync_v3 PRIVATE v3)
target_link_libraries(sync_v3 PRIVATE Threads::Threads)
