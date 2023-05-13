add_library(unstately INTERFACE)
target_include_directories(unstately INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)
target_include_directories(unstately INTERFACE $<INSTALL_INTERFACE:include>)

install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/unstately"
    DESTINATION "include"
)

install(TARGETS unstately
    EXPORT unstately-targets
)

install(EXPORT unstately-targets
    NAMESPACE unstately::
    FILE unstately-config.cmake
    DESTINATION lib/cmake/unstately
)

add_library(unstately::unstately ALIAS unstately)
