# v1 — the interface and an in-memory implementation, checked by a contract.
add_chapter_test(contracts_v1 v1/memory_store.c v1/contract.c v1/store_test.c)
target_include_directories(contracts_v1 PRIVATE v1)

# v2 — a file-backed implementation must satisfy the same contract.
add_chapter_test(contracts_v2 v2/memory_store.c v2/file_store.c v2/contract.c
                                 v2/store_test.c)
target_include_directories(contracts_v2 PRIVATE v2)
