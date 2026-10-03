# v1 — mutate a struct through a pointer.
add_chapter_test(pointers_errors_v1 v1/wallet.c v1/wallet_test.c)
target_include_directories(pointers_errors_v1 PRIVATE v1)

# v2 — return error codes instead of throwing.
add_chapter_test(pointers_errors_v2 v2/wallet.c v2/wallet_test.c)
target_include_directories(pointers_errors_v2 PRIVATE v2)
