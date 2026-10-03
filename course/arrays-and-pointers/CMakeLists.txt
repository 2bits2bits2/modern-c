# v1 — sum an array (which is really a pointer and a length).
add_chapter_test(arrays_v1 v1/sum.c v1/sum_test.c)
target_include_directories(arrays_v1 PRIVATE v1)

# v2 — a growable array built on malloc and realloc.
add_chapter_test(arrays_v2 v2/ints.c v2/ints_test.c)
target_include_directories(arrays_v2 PRIVATE v2)
