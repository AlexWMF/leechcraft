cmake_minimum_required (VERSION 2.8)
project (leechcraft_pogooglue)
include (InitLCPlugin OPTIONAL)

include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

set (SRCS
	pogooglue.cpp
	)
set (RESOURCES pogooglueresources.qrc)

CreateTrs ("pogooglue" "en;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("pogooglue" "en;ru_RU" "${SRCS}" "" "")

QT4_ADD_RESOURCES (RCCS ${RESOURCES})

add_library (leechcraft_pogooglue SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${RCCS}
	)
target_link_libraries (leechcraft_pogooglue
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_pogooglue DESTINATION ${LC_PLUGINS_DEST})
