# v1 — the error struct and a service that uses it.
add_chapter_test(error_types_v1 v1/users_test.c users.c error.c)
target_include_directories(error_types_v1 PRIVATE v1 ${CMAKE_CURRENT_SOURCE_DIR})

# v2 — mapping errors onto HTTP responses.
add_chapter_test(error_types_v2 v2/http_error_test.c v2/http_error.c users.c
                                 error.c)
target_include_directories(error_types_v2 PRIVATE v2
                                               ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(error_types_v2 PRIVATE app)
