# The shared echo server for this chapter's acceptance tests.
add_chapter_program(scaling_echo_server echo_server.c)

# v1 — raw lifecycle management in the test.
add_acceptance_test(scaling_v1 scaling_echo_server v1/scaling_test.c)

# v2 — the same thing behind a fixture, plus a stress loop.
add_acceptance_test(scaling_v2 scaling_echo_server v2/fixture.c v2/scaling_test.c)
