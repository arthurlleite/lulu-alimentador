FILE(GLOB app_sources ${CMAKE_SOURCE_DIR}/src/*.c)

# O app web (pasta ../web) vai embutido na flash: a ESP32 serve a mesma página
# do GitHub Pages em http://alimentador.local, mesmo sem internet.
idf_component_register(SRCS ${app_sources}
                       INCLUDE_DIRS "."
                       EMBED_TXTFILES ${CMAKE_SOURCE_DIR}/../web/index.html
                                      ${CMAKE_SOURCE_DIR}/../web/setup.html)
