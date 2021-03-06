if (WITH_QWT)
	find_package (Qwt REQUIRED)
	set (ADDITIONAL_LIBS ${QWT_LIBRARIES})
	add_definitions (-DWITH_QWT)

	include_directories (${QWT_INCLUDE_DIRS})
endif ()

set (QML_SRCS
	colorthemeproxy.cpp
	settableiconprovider.cpp
	standardnamfactory.cpp
	themeimageprovider.cpp
	tooltipitem.cpp
	unhidelistmodel.cpp
	unhidelistviewbase.cpp
	widthiconprovider.cpp
	)

if (WITH_QWT)
	set (QML_SRCS ${QML_SRCS} plotitem.cpp)
endif ()

foreach (SRC ${QML_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/qml/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

add_library (leechcraft-util-qml SHARED
	${QML_SRCS}
	)
target_link_libraries (leechcraft-util-qml
	${ADDITIONAL_LIBS}
	leechcraft-util-gui
	leechcraft-util-network
	leechcraft-util-sys
	)
set_property (TARGET leechcraft-util-qml PROPERTY SOVERSION ${LC_SOVERSION})
install (TARGETS leechcraft-util-qml DESTINATION ${LIBDIR})

FindQtLibs (leechcraft-util-qml Network Quick QuickWidgets)
