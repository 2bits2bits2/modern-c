# v1 — type dispatch and typeof in macros.
add_chapter_test(generics_v1 v1/generic_test.c)
target_include_directories(generics_v1 PRIVATE v1)

# v2 — a macro that defines a whole typed collection.
add_chapter_test(generics_v2 v2/generic_array_test.c)
target_include_directories(generics_v2 PRIVATE v2)
