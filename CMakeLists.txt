set(srcs
    src/BMP180.cpp
    src/oled/OLEDDisplay.cpp
    src/oled/OLEDDisplayUi.cpp
    src/heltec.cpp
    src/lora/LoRa.cpp
  )

set(includedirs
    src
    src/lora
    src/oled
  )

set(priv_includes )
set(requires arduino)
set(priv_requires )

idf_component_register(INCLUDE_DIRS ${includedirs} PRIV_INCLUDE_DIRS ${priv_includes} SRCS ${srcs} REQUIRES ${requires} PRIV_REQUIRES ${priv_requires})

function(maybe_add_component component_name)
    idf_build_get_property(components BUILD_COMPONENTS)
    if (${component_name} IN_LIST components)
        idf_component_get_property(lib_name ${component_name} COMPONENT_LIB)
        target_link_libraries(${COMPONENT_LIB} PUBLIC ${lib_name})
    endif()
endfunction()

maybe_add_component(arduino)

target_compile_options(${COMPONENT_TARGET} PUBLIC
    -DESP32)
