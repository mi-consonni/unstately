find_package(Doxygen COMPONENTS dot)

if(DOXYGEN_FOUND)
    set(DOXYGEN_PROJECT_NAME "Unstately")
    set(DOXYGEN_PROJECT_NUMBER "${UNSTATELY_VERSION}")

    set(DOXYGEN_EXTENSION_MAPPING "no_extension=md")
    set(DOXYGEN_EXTRACT_ALL YES)
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "README.md")
    set(DOXYGEN_VERBATIM_HEADERS NO)

    doxygen_add_docs(unstately-doxygen
        README.md LICENSE docs include
        ALL
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        COMMENT "Generate Doxygen documentation"
    )

    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html
        DESTINATION share/doc/${PROJECT_NAME}
    )
else()
    message(WARNING "Skipping generation of Doxygen documentation")
endif()
