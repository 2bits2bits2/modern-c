# v1 — the domain unit test and the acceptance test.
add_chapter_test(acceptance_v1_unit v1/greeting.c v1/greeting_test.c)
target_include_directories(acceptance_v1_unit PRIVATE v1)

# v1 — the server program.
add_chapter_program(acceptance_v1_server v1/greeting.c v1/server.c)
target_include_directories(acceptance_v1_server PRIVATE v1)

# v1 — the acceptance test drives the server program.
add_acceptance_test(acceptance_v1 acceptance_v1_server v1/acceptance_test.c)
target_include_directories(acceptance_v1 PRIVATE v1)

# v2 — a server that shuts down gracefully.
add_chapter_program(acceptance_v2_server v2/greeting.c v2/server.c)
target_include_directories(acceptance_v2_server PRIVATE v2)

add_acceptance_test(acceptance_v2 acceptance_v2_server v2/acceptance_test.c)
target_include_directories(acceptance_v2 PRIVATE v2)
