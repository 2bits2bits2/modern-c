# v1 — wait on two file descriptors with poll().
add_chapter_test(select_v1 v1/ready.c v1/ready_test.c)
target_include_directories(select_v1 PRIVATE v1)
