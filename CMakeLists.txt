cmake_minimum_required(VERSION 4.1)
project(ivpn)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 20)
set(QT_FORCE_CMP0156_TO_VALUE NEW)
find_package(Qt6 COMPONENTS
        Core Widgets Gui    OpenGL Qml  Quick
        QuickControls2
        OpenGLWidgets OpenGL Multimedia test  Network
        Concurrent
        REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(ZXing REQUIRED)
find_package(OpenSSL REQUIRED)



qt_policy(SET QTP0001 NEW)
qt_add_executable(${PROJECT_NAME}
        main.cpp
        core/NodeParser.cpp
        core/NodeParser.h
        core/QRCodeGen.cpp
        core/QRCodeGen.h
        Backend.cpp
        Backend.h
        core/node.cpp
        core/node.h
        core/QRProvider.cpp
        core/QRProvider.h
        core/http_utils.cpp
        core/http_utils.h
        core/NodeModel.cpp
        core/NodeModel.h
)

qt_add_qml_module(${PROJECT_NAME}
        URI ${PROJECT_NAME}
        VERSION 1.0
        QML_FILES
        qml/Main.qml
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE Qt6::Quick
        ZXing::ZXing
        Qt6::Concurrent
        Qt6::Network
        yaml-cpp::yaml-cpp
        OpenSSL::SSL OpenSSL::Crypto
)

if (MSVC)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms

            COMMAND ${CMAKE_COMMAND} -E copy
            #            "${_VCPKG_INSTALLED_DIR}/x64-windows/tools/Qt6/plugins/platforms/qwindows$<$<CONFIG:Debug>:d>.dll"
            "${_VCPKG_INSTALLED_DIR}/x64-windows/$<$<CONFIG:Debug>:debug/>/Qt6/plugins/platforms/qwindows$<$<CONFIG:Debug>:d>.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms"
    )
endif()
