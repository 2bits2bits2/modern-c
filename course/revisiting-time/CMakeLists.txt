# v1 — a whole tournament simulated on a virtual clock.
add_chapter_test(revisiting_time_v1 v1/tournament.c v1/tournament_test.c)
target_include_directories(revisiting_time_v1 PRIVATE v1)
target_link_libraries(revisiting_time_v1 PRIVATE app)
