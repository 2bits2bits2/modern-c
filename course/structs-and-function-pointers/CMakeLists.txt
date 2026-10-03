# v1 — a plain struct with functions that take it by value.
add_chapter_test(structs_v1 v1/rectangle.c v1/rectangle_test.c)
target_include_directories(structs_v1 PRIVATE v1)

# v2 — "interfaces" via a vtable of function pointers.
add_chapter_test(structs_v2 v2/shapes.c v2/shapes_test.c)
target_include_directories(structs_v2 PRIVATE v2)
