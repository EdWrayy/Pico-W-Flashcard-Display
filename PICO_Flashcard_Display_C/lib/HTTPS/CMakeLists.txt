# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_ePaper_SRCS 变量
aux_source_directory(. DIR_HTTPS_SRCS)

include_directories(../Config)

# Add HTTPS static library
add_library(HTTPS ${DIR_HTTPS_SRCS})

# Link with Config (if needed) and required Pico/Net libraries
target_link_libraries(HTTPS
    PUBLIC
        Config
        pico_stdlib
        pico_stdio_usb
        pico_cyw43_arch_lwip_threadsafe_background
        pico_lwip_mbedtls
        pico_mbedtls
)

# Add include directories needed for TLS
target_include_directories(HTTPS PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${PROJECT_SOURCE_DIR}  
    ${PICO_SDK_PATH}/lib/lwip/src/include
    ${PICO_LWIP_PATH}/src/apps/altcp_tls
    ${PICO_SDK_PATH}/lib/mbedtls/include
)