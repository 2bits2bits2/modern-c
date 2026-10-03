# v1 — count down, with an injected writer and a spy sleeper.
add_chapter_test(mocking_v1 v1/countdown.c v1/writer.c v1/buffer.c v1/countdown_test.c)
target_include_directories(mocking_v1 PRIVATE v1)
