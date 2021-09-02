set(CPACK_PACKAGE_INSTALL_DIRECTORY     Graphviz                        )
set(CPACK_PACKAGE_NAME                  ${PROJECT_NAME}                 )
SET(CPACK_PACKAGE_VERSION_MAJOR         ${GRAPHVIZ_VERSION_MAJOR}       )
SET(CPACK_PACKAGE_VERSION_MINOR         ${GRAPHVIZ_VERSION_MINOR}       )
SET(CPACK_PACKAGE_VERSION_PATCH         ${GRAPHVIZ_VERSION_PATCH}       )
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY   "Graph Visualization Tools"     )
set(CPACK_PACKAGE_VENDOR                Graphviz                        )
set(CPACK_PACKAGE_CONTACT               http://www.graphviz.org/        )
set(CPACK_RESOURCE_FILE_LICENSE         "${TOP_SOURCE_DIR}/LICENSE"     )
set(CPACK_RESOURCE_FILE_README          "${TOP_SOURCE_DIR}/README.md"   )

set(CPACK_GENERATOR ZIP)

if(CYGWIN)
    LIST(APPEND CPACK_GENERATOR CygwinBinary)
elseif (UNIX)
    if (NOT APPLE)
        if (EXISTS "/etc/os-release")
            file(READ "/etc/os-release" OS_RELEASE)
            if ("${OS_RELEASE}" MATCHES "ID_LIKE=debian")
                LIST(APPEND CPACK_GENERATOR DEB)
            else()
                LIST(APPEND CPACK_GENERATOR RPM)
            endif()
        endif()
    endif()
endif()

find_package(NSIS)
if(NSIS_FOUND)
    set(CPACK_NSIS_MUI_ICON                 "${TOP_SOURCE_DIR}/windows/build/Graphviz.ico"  )
    set(CPACK_NSIS_MUI_UNIICON              "${TOP_SOURCE_DIR}/windows/build/Graphviz.ico"  )
    set(CPACK_NSIS_INSTALLED_ICON_NAME      "Uninstall.exe"                                 )
    set(CPACK_NSIS_HELP_LINK                "http://www.graphviz.org"                       )
    set(CPACK_NSIS_URL_INFO_ABOUT           "http://www.graphviz.org"                       )
    set(CPACK_NSIS_MODIFY_PATH              ON                                              )
    if(CMAKE_CL_64)
        set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
    else()
        set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
    endif()
    set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
        Delete \\\"${CPACK_NSIS_INSTALL_ROOT}\\\\${CPACK_PACKAGE_INSTALL_DIRECTORY}\\\\${BINARY_INSTALL_DIR}\\\\config6\\\"
    ")
    LIST(APPEND CPACK_GENERATOR NSIS)
endif()
