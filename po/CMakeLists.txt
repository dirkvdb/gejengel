CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

FIND_PACKAGE(Gettext)
IF (GETTEXT_FOUND)
    OPTION(HAVE_GETTEXT "Translation support" ON)
    FIND_PROGRAM(INTLTOOL_EXTRACT intltool-extract)
    FIND_PROGRAM(INTLTOOL_UPDATE intltool-update)
    FIND_PROGRAM(GETTEXT_MSGMERGE_EXECUTABLE msgmerge)
    FIND_PROGRAM(GETTEXT_MSGFMT_EXECUTABLE msgfmt)
    
    SET(POTFILE ${PACKAGE}.pot)
    SET(POFILES nl.po es.po)
    SET(INSTALL_LOC_DIR share/locale)

    ADD_CUSTOM_COMMAND(
        OUTPUT ${POTFILE}
        COMMAND INTLTOOL_EXTRACT=${INTLTOOL_EXTRACT} srcdir=${CMAKE_CURRENT_SOURCE_DIR} ${INTLTOOL_UPDATE} ARGS --gettext-package ${PACKAGE} --pot
        COMMENT "Generating pot file"
    )

    FOREACH (_currentPoFile ${POFILES})
        GET_FILENAME_COMPONENT(_absFile ${_currentPoFile} ABSOLUTE)
        GET_FILENAME_COMPONENT(_abs_PATH ${_absFile} PATH)
        GET_FILENAME_COMPONENT(_lang ${_absFile} NAME_WE)
        SET(_gmoFilePath "${CMAKE_CURRENT_BINARY_DIR}/${_lang}/LC_MESSAGES")
        SET(_gmoFile "${_gmoFilePath}/${PACKAGE}.mo")

        FILE(MAKE_DIRECTORY "${_gmoFilePath}")
        ADD_CUSTOM_COMMAND( 
            OUTPUT ${_gmoFile} 
            COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} --quiet --update --backup=none -s ${_absFile} ${CMAKE_CURRENT_BINARY_DIR}/${POTFILE}
            COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} -o ${_gmoFile} ${_absFile}
            DEPENDS ${PACKAGE}.pot ${_absFile} 
        )

        INSTALL(FILES ${_gmoFile} DESTINATION ${INSTALL_LOC_DIR}/${_lang}/LC_MESSAGES RENAME ${PACKAGE}.mo COMPONENT "locales")
        LIST(APPEND _gmoFiles ${_gmoFile})
    ENDFOREACH (_currentPoFile)

    ADD_CUSTOM_TARGET(translations ${_addToAll} DEPENDS ${POTFILE} ${_gmoFiles})

    ADD_DEPENDENCIES(gejengel translations)
ENDIF (GETTEXT_FOUND)
