# Disable annoying MSVC warnings in rgb2spec build
if (MSVC)
  add_definitions(/D "_CRT_SECURE_NO_WARNINGS")
endif()

# build rgb2spec
add_subdirectory(rgb2spec)
set(RGB2SPEC_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/rgb2spec PARENT_SCOPE)
set_property(TARGET rgb2spec         PROPERTY FOLDER "dependencies")
set_property(TARGET rgb2spec_opt     PROPERTY FOLDER "dependencies")
set_property(TARGET rgb2spec_opt_run PROPERTY FOLDER "dependencies")