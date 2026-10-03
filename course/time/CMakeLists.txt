# v1 — the current blind level, from an injected clock.
add_chapter_test(time_v1 v1/blind.c v1/blind_test.c)
target_include_directories(time_v1 PRIVATE v1)
target_link_libraries(time_v1 PRIVATE app)

# v2 — a scheduler that fires callbacks when their time arrives.
add_chapter_test(time_v2 v2/scheduler_test.c)
target_link_libraries(time_v2 PRIVATE app)
