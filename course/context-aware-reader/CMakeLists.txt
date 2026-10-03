# v1 — a cancellable read over a file descriptor.
add_chapter_test(context_reader_v1 v1/reader_test.c reader.c)
target_include_directories(context_reader_v1
                           PRIVATE v1 ${CMAKE_CURRENT_SOURCE_DIR})

# v2 — copying, interrupted by cancellation from another thread.
add_chapter_test(context_reader_v2 v2/reader_test.c reader.c)
target_include_directories(context_reader_v2
                           PRIVATE v2 ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(context_reader_v2 PRIVATE Threads::Threads)
