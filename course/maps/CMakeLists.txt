# v1 — a hash map with chaining.
add_chapter_test(maps_v1 v1/map.c v1/map_test.c)
target_include_directories(maps_v1 PRIVATE v1)

# v2 — deletion and resizing.
add_chapter_test(maps_v2 v2/map.c v2/map_test.c)
target_include_directories(maps_v2 PRIVATE v2)
