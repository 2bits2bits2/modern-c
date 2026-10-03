# v1 — the store, unit tested.
add_chapter_test(http_server_v1_store v1/player_store_test.c)
target_include_directories(http_server_v1_store PRIVATE v1)
target_link_libraries(http_server_v1_store PRIVATE app)

# v1 — the handler, unit tested.
add_chapter_test(http_server_v1_handler v1/handler_test.c handler.c)
target_include_directories(http_server_v1_handler PRIVATE v1
                                                          ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(http_server_v1_handler PRIVATE app)

# v2 — the whole server, driven over TCP by an acceptance test.
add_chapter_program(http_server_v2_server v2/main.c handler.c)
target_include_directories(http_server_v2_server PRIVATE v2
                                                      ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(http_server_v2_server PRIVATE app)

add_acceptance_test(http_server_v2 http_server_v2_server v2/acceptance_test.c)
target_link_libraries(http_server_v2 PRIVATE app)
