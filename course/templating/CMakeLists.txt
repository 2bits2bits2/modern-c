# v1 — replace a single {key}.
add_chapter_test(templating_v1 v1/render.c v1/render_test.c)
target_include_directories(templating_v1 PRIVATE v1)

# v2 — replace any number of keys from a table of bindings.
add_chapter_test(templating_v2 v2/template.c v2/template_test.c)
target_include_directories(templating_v2 PRIVATE v2)
