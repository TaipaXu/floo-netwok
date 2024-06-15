import QtQuick
import QtQuick.Controls

MenuBar {
    id: root

    signal requestCreateChannel
    signal requestJoinChannel
    signal requestShowSettings

    Menu {
        title: qsTr("&File")

        MenuItem {
            action: Action {
                icon.name: "document-new"
                text: qsTr("&Create a channel")

                onTriggered: root.requestCreateChannel()
            }
        }

        MenuItem {
            action: Action {
                icon.name: "document-open"
                text: qsTr("&Join a channel")

                onTriggered: root.requestJoinChannel()
            }
        }

        MenuItem {
            action: Action {
                icon.name: "preferences-system"
                text: qsTr("&Settings")

                onTriggered: root.requestShowSettings();
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

                onTriggered: utils.openMyGithubPage()
            }
        }

        MenuItem {
            action: Action {
                icon.name: "help-about"
                text: qsTr("&Project domain")

                onTriggered: utils.openThisProjectPage()
            }
        }

        MenuItem {
            action: Action {
                icon.name: "help-about"
                text: qsTr("&Bug report")

                onTriggered: utils.openThisProjectIssuesPage()
            }
        }
    }
}
