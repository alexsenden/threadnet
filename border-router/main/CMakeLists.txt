idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS ".")

idf_build_get_property(python PYTHON)
    add_custom_target(rcp_image_generation ALL
        COMMAND ${python} ${CMAKE_CURRENT_SOURCE_DIR}/../create_ota_image.py
        --rcp-build-dir $ENV{IDF_PATH}/examples/openthread/ot_rcp/build
        --target-file ${CMAKE_CURRENT_BINARY_DIR}/spiffs_image/ot_rcp_0/rcp_image
        )

    spiffs_create_partition_image(rcp_fw ${CMAKE_CURRENT_BINARY_DIR}/spiffs_image FLASH_IN_PROJECT
                                    DEPENDS rcp_image_generation)
