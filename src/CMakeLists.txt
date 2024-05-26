find_package(Qt6 REQUIRED COMPONENTS Core Widgets Quick QuickControls2 Qml Network WebSockets HttpServer Multimedia)

configure_file(./appConfig.hpp.in ./appConfig.hpp)
configure_file(assets.qrc assets.qrc @ONLY)

if(APPLE)
    set(CMAKE_MACOSX_BUNDLE ON)
endif()

set(TARGET floo-network)
set(SOURCES
        main.cpp
        core.hpp
        core.cpp

        models/channel.hpp
        models/channel.cpp
        models/file.hpp
        models/file.cpp
        models/myFile.hpp
        models/myFile.cpp
        models/connection.hpp
        models/connection.cpp

        network/server.hpp
        network/server.cpp
        network/client.hpp
        network/client.cpp
        network/sendManager.hpp
        network/sendManager.cpp
        network/receiveManager.hpp
        network/receiveManager.cpp
        network/sender.hpp
        network/sender.cpp
        network/receiver.hpp
        network/receiver.cpp
        network/webServer.hpp
        network/webServer.cpp

        utils/utils.hpp
        utils/utils.cpp

        assets.qrc
        web.qrc
)

add_executable(
    ${TARGET}
    ${SOURCES}
)

if(UNIX AND NOT APPLE)
add_custom_command(TARGET ${TARGET} POST_BUILD
        COMMAND strip $<TARGET_FILE:floo-network>
        COMMENT "Stripping executable $<TARGET_FILE:floo-network>")
endif()

target_link_libraries(${TARGET} PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::WebSockets
    Qt${QT_VERSION_MAJOR}::HttpServer
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::QuickControls2
)

qt_add_qml_module(${TARGET}
    URI qml
    NO_RESOURCE_TARGET_PATH
    QML_FILES
        "widgets/MainWindow.qml"
        "widgets/Tray.qml"
        "widgets/MainWindowMenuBar.qml"
        "widgets/Navbar.qml"
        "widgets/NavButton.qml"
        "widgets/Home.qml"
        "widgets/CreateChannelDialog.qml"
        "widgets/JoinChannelDialog.qml"
        "widgets/ClientChannel.qml"
        "widgets/ServerChannel.qml"
        "widgets/Tag.qml"
        "widgets/MyFile.qml"
        "widgets/File.qml"
        "widgets/Sends.qml"
        "widgets/Receives.qml"
        "widgets/Sender.qml"
        "widgets/Receiver.qml"
)
