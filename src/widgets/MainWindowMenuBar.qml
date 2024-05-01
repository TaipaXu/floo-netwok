import QtQuick
import QtQuick.Controls

MenuBar {
    signal requestCreateChannel
    signal requestJoinChannel
    signal requestShowUploader
    signal requestShowDownloader

    Menu {
        title: qsTr("&File")

        MenuItem {
            action: Action {
                icon.name: "document-new"
                text: qsTr("&Create a channel")

                onTriggered: requestCreateChannel()
            }
        }

        MenuItem {
            action: Action {
                icon.name: "document-open"
                text: qsTr("&Join a channel")

                onTriggered: requestJoinChannel()
            }
        }

        MenuItem {
            action: Action {
                icon.name: "preferences-system"
                text: qsTr("&Settings")
            }
        }

        MenuSeparator {}

        MenuItem {
            action: Action {
                icon.name: "application-exit"
                text: qsTr("&Quit")
                onTriggered: Qt.quit()
            }
        }
    }

    Menu {
        title: qsTr("&About")

        MenuItem {
            action: Action {
                icon.name: "help-about"
                text: qsTr("&About me")

                onTriggered: Qt.openUrlExternally("https://github.com/TaipaXu")
            }
        }

        MenuItem {
            action: Action {
                icon.name: "help-about"
                text: qsTr("&Project domain")

                onTriggered: Qt.openUrlExternally("https://github.com/TaipaXu/floo-network")
            }
        }

        MenuItem {
            action: Action {
                icon.name: "help-about"
                text: qsTr("&Bug report")

                onTriggered: Qt.openUrlExternally("https://github.com/TaipaXu/floo-network/issues")
            }
        }
    }
}
