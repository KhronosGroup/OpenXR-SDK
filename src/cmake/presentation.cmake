set(PRESENTATION_BACKENDS xlib xcb wayland)
set(PRESENTATION_BACKEND xlib CACHE STRING
    "Presentation backend chosen at configure time")
set_property(CACHE PRESENTATION_BACKEND PROPERTY STRINGS
                ${PRESENTATION_BACKENDS})

list(FIND PRESENTATION_BACKENDS ${PRESENTATION_BACKEND} index)
if(index EQUAL -1)
    message(FATAL_ERROR "Presentation backend must be one of
            ${PRESENTATION_BACKENDS}")
endif()

message(STATUS "Using presentation backend: ${PRESENTATION_BACKEND}")


if( PRESENTATION_BACKEND MATCHES "xlib" )
    find_package(X11 REQUIRED)
    if ((NOT X11_Xxf86vm_LIB) OR (NOT X11_Xrandr_LIB))
        message(FATAL_ERROR "OpenXR xlib backend requires Xxf86vm and Xrandr")
    endif()

    add_definitions( -DSUPPORT_X )
    add_definitions( -DOS_LINUX_XLIB )
    set( XLIB_LIBRARIES
            ${X11_LIBRARIES}
            ${X11_Xxf86vm_LIB}
            ${X11_Xrandr_LIB} )

elseif( PRESENTATION_BACKEND MATCHES "xcb" )
    find_package(PkgConfig REQUIRED)
    # XCB + XCB GLX is limited to OpenGL 2.1
    # add_definitions( -DOS_LINUX_XCB )
    # XCB + Xlib GLX 1.3
    add_definitions( -DOS_LINUX_XCB_GLX )

    pkg_search_module(X11 REQUIRED x11)
    pkg_search_module(XCB REQUIRED xcb)
    pkg_search_module(XCB_RANDR REQUIRED xcb-randr)
    pkg_search_module(XCB_KEYSYMS REQUIRED xcb-keysyms)
    pkg_search_module(XCB_GLX REQUIRED xcb-glx)
    pkg_search_module(XCB_DRI2 REQUIRED xcb-dri2)
    pkg_search_module(XCB_ICCCM REQUIRED xcb-icccm)

    set( XCB_LIBRARIES
            ${XCB_LIBRARIES}
            ${XCB_KEYSYMS_LIBRARIES}
            ${XCB_RANDR_LIBRARIES}
            ${XCB_GLX_LIBRARIES}
            ${XCB_DRI2_LIBRARIES}
            ${X11_LIBRARIES} )

elseif( PRESENTATION_BACKEND MATCHES "wayland" )
    find_package(PkgConfig REQUIRED)
    pkg_search_module(WAYLAND_CLIENT REQUIRED wayland-client)
    pkg_search_module(WAYLAND_EGL REQUIRED wayland-egl)
    pkg_search_module(WAYLAND_SCANNER REQUIRED wayland-scanner)
    pkg_search_module(WAYLAND_PROTOCOLS REQUIRED wayland-protocols>=1.7)
    pkg_search_module(EGL REQUIRED egl)

    add_definitions( -DOS_LINUX_WAYLAND )
    set( WAYLAND_LIBRARIES
            ${EGL_LIBRARIES}
            ${WAYLAND_CLIENT_LIBRARIES}
            ${WAYLAND_EGL_LIBRARIES} )

    # generate wayland protocols
    set(WAYLAND_PROTOCOLS_DIR ${PROJECT_SOURCE_DIR}/wayland-protocols/)
    file(MAKE_DIRECTORY ${WAYLAND_PROTOCOLS_DIR})

    pkg_get_variable(WAYLAND_PROTOCOLS_DATADIR wayland-protocols pkgdatadir)
    pkg_get_variable(WAYLAND_SCANNER wayland-scanner wayland_scanner)

    set(PROTOCOL xdg-shell-unstable-v6)
    set(PROTOCOL_XML
        ${WAYLAND_PROTOCOLS_DATADIR}/unstable/xdg-shell/${PROTOCOL}.xml)

    if( EXISTS ${PROTOCOL_XML} )
        execute_process(COMMAND
                        ${WAYLAND_SCANNER}
                        code
                        ${PROTOCOL_XML}
                        ${WAYLAND_PROTOCOLS_DIR}/${PROTOCOL}.c)
        execute_process(COMMAND
                        ${WAYLAND_SCANNER}
                        client-header
                        ${PROTOCOL_XML}
                        ${WAYLAND_PROTOCOLS_DIR}/${PROTOCOL}.h)

        set( WAYLAND_PROTOCOL_SRC
                ${WAYLAND_PROTOCOLS_DIR}/${PROTOCOL}.c
                ${WAYLAND_PROTOCOLS_DIR}/${PROTOCOL}.h )

        include_directories(${WAYLAND_PROTOCOLS_DIR})
    else()
        message(FATAL_ERROR
                "xdg-shell-unstable-v6.xml not found in "
                ${WAYLAND_PROTOCOLS_DATADIR}
                "\nYour wayland-protocols package does not "
                "contain xdg-shell-unstable-v6.")
    endif()
endif()
