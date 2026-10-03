# v1 — the encoder and the minimal parser.
add_chapter_test(json_v1 v1/json_test.c)
target_link_libraries(json_v1 PRIVATE app)

# v2 — the JSON-speaking handler, unit tested.
add_chapter_test(json_v2_handler v2/handler_test.c handler.c)
target_include_directories(json_v2_handler PRIVATE v2 ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(json_v2_handler PRIVATE app)

# v2 — the server, driven over HTTP.
add_chapter_program(json_v2_server v2/main.c handler.c)
target_include_directories(json_v2_server PRIVATE v2 ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(json_v2_server PRIVATE app)

add_acceptance_test(json_v2 json_v2_server v2/acceptance_test.c)
target_link_libraries(json_v2 PRIVATE app)
