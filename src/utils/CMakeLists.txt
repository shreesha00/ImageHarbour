set(UTILS_SRC ${CMAKE_CURRENT_SOURCE_DIR}/properties.cc)
target_sources(imagesvr PRIVATE ${UTILS_SRC})
target_sources(imageharbourcli PRIVATE ${UTILS_SRC})