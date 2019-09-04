
set(GOTCHA_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/gotcha/gotcha.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gotcha/gotcha_types.h)

#target_sources(gotcha-include INTERFACE ${GOTCHA_HEADERS})

install(DIRECTORY gotcha DESTINATION include FILES_MATCHING PATTERN *.h)
