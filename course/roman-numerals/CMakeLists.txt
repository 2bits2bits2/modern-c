# v1 — Roman numerals, and a property-based round trip.
add_chapter_test(roman_v1 v1/roman.c v1/roman_test.c)
target_include_directories(roman_v1 PRIVATE v1)
