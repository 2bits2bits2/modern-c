# v1 — option parsing, unit tested.
add_chapter_test(command_line_v1_options v1/options_test.c options.c)
target_include_directories(command_line_v1_options
                           PRIVATE v1 ${CMAKE_CURRENT_SOURCE_DIR})

# v1 — the server, configured from the command line.
add_chapter_program(command_line_server v1/main.c options.c handler.c)
target_include_directories(command_line_server
                           PRIVATE v1 ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(command_line_server PRIVATE app)

# v2 — the CLI, a second program in the same codebase.
add_chapter_program(command_line_cli v2/cli.c)
target_link_libraries(command_line_cli PRIVATE app)

# v2 — the CLI talks to the server.
add_acceptance_test(command_line_v2 command_line_server v2/acceptance_test.c)
target_compile_definitions(command_line_v2
                           PRIVATE CLI_PATH="$<TARGET_FILE:command_line_cli>")
target_link_libraries(command_line_v2 PRIVATE app)
