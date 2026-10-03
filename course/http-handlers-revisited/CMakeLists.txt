# v1 — a declarative router replacing the if/else chain.
add_chapter_test(http_revisited_v1 v1/handler_test.c handler.c)
target_include_directories(http_revisited_v1
                           PRIVATE v1 ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(http_revisited_v1 PRIVATE app)

# v2 — logging middleware around the handler.
add_chapter_test(http_revisited_v2 v2/middleware_test.c v2/middleware.c)
target_include_directories(http_revisited_v2
                           PRIVATE v1 v2 ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(http_revisited_v2 PRIVATE app)

add_chapter_program(http_revisited_server v2/main.c handler.c v2/middleware.c)
target_include_directories(http_revisited_server
                           PRIVATE v1 v2 ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(http_revisited_server PRIVATE app)

add_acceptance_test(http_revisited_acceptance http_revisited_server
                    v2/acceptance_test.c)
target_include_directories(http_revisited_acceptance
                           PRIVATE v1 v2 ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(http_revisited_acceptance PRIVATE app)
