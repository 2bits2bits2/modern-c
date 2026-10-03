# v1 — read a whole file into a heap buffer.
add_chapter_test(reading_files_v1 v1/read_file.c v1/read_file_test.c)
target_include_directories(reading_files_v1 PRIVATE v1)
