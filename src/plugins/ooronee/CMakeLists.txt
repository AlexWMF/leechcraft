cmake_minimum_required (VERSION 2.8)
project (leechcraft_ooronee)
include (InitLCPlugin OPTIONAL)

set (QT_USE_QTDECLARATIVE TRUE)
include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	ooronee.cpp
	droparea.cpp
	quarkproxy.cpp
	xmlsettingsmanager.cpp
	)
#CreateTrs("ooronee" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("ooronee" "en;ru_RU" "${SRCS}" "${FORMS}" "")

add_library (leechcraft_ooronee SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_ooronee
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_ooronee DESTINATION ${LC_PLUGINS_DEST})
install (DIRECTORY share/qml/ooronee DESTINATION ${LC_QML_DEST})
install (FILES ooroneesettings.xml DESTINATION ${LC_SETTINGS_DEST})
