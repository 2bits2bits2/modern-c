# v1 — type dispatch at compile time with _Generic.
add_chapter_test(reflection_v1 v1/describe_test.c)
target_include_directories(reflection_v1 PRIVATE v1)

# v2 — a tagged union, the C way to model a value of several types.
add_chapter_test(reflection_v2 v2/value.c v2/value_test.c)
target_include_directories(reflection_v2 PRIVATE v2)
