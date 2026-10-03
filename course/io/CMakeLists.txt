# v1 — persistence and sorting, unit tested.
add_chapter_test(io_v1 v1/persistence_test.c)
target_link_libraries(io_v1 PRIVATE app)

# v2 — the handler persists, and the league is sorted.
add_chapter_test(io_v2_handler v2/handler_test.c handler.c)
target_include_directories(io_v2_handler PRIVATE v2 ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(io_v2_handler PRIVATE app)

# v2 — the server, with data that survives a restart.
add_chapter_program(io_v2_server v2/main.c handler.c)
target_include_directories(io_v2_server PRIVATE v2 ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(io_v2_server PRIVATE app)

add_acceptance_test(io_v2 io_v2_server v2/acceptance_test.c)
target_link_libraries(io_v2 PRIVATE app)
