
set( INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" )

set( SOURCES
    ${INCLUDE_DIR}/include.c )
set( GL_HEADERS
    ${INCLUDE_DIR}/GL/gl_format.h
    ${INCLUDE_DIR}/GL/glext.h
    ${INCLUDE_DIR}/GL/wglext.h )
set( UTILS_HEADERS
    ${INCLUDE_DIR}/utils/algebra.h
    ${INCLUDE_DIR}/utils/base64.h
    ${INCLUDE_DIR}/utils/json.h
    ${INCLUDE_DIR}/utils/lexer.h
    ${INCLUDE_DIR}/utils/sysinfo.h
    ${INCLUDE_DIR}/utils/nanoseconds.h
    ${INCLUDE_DIR}/utils/threading.h )
set( VULKAN_HEADERS
    ${INCLUDE_DIR}/vulkan/vk_format.h )
set( D3D_HEADERS
    ${INCLUDE_DIR}/d3d/d3d_format.h )
set( METAL_HEADERS
    ${INCLUDE_DIR}/metal/mtl_format.h )
source_group( Assemble FILES ${SOURCES} )
source_group( utils FILES ${UTILS_HEADERS} )
source_group( GL FILES ${GL_HEADERS} )
source_group( vulkan FILES ${VULKAN_HEADERS} )
source_group( d3d FILES ${D3D_HEADERS} )
source_group( metal FILES ${METAL_HEADERS} )
add_library( include STATIC ${SOURCES} ${UTILS_HEADERS} ${GL_HEADERS} ${VULKAN_HEADERS} ${D3D_HEADERS} ${METAL_HEADERS} )
set_property( TARGET include PROPERTY FOLDER external/include )
