# v1 — Base64, SHA-1 and frame encoding, against known vectors.
add_chapter_test(websockets_v1_base64 v1/base64_test.c)
target_link_libraries(websockets_v1_base64 PRIVATE app)

add_chapter_test(websockets_v1_sha1 v1/sha1_test.c)
target_link_libraries(websockets_v1_sha1 PRIVATE app)

add_chapter_test(websockets_v1_frames v1/websocket_test.c)
target_link_libraries(websockets_v1_frames PRIVATE app)

# v2 — a WebSocket echo server, driven through a real handshake.
add_chapter_program(websockets_v2_server v2/server.c)
target_link_libraries(websockets_v2_server PRIVATE app)

add_acceptance_test(websockets_v2 websockets_v2_server v2/acceptance_test.c)
target_link_libraries(websockets_v2 PRIVATE app)
