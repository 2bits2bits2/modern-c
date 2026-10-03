# v1 — trigonometry behind a clock.
add_chapter_test(math_v1 v1/hand.c v1/hand_test.c)
target_include_directories(math_v1 PRIVATE v1)

# v2 — render a clock face as SVG.
add_chapter_program(math_v2_clock v1/hand.c v2/clock.c)
target_include_directories(math_v2_clock PRIVATE v1)
