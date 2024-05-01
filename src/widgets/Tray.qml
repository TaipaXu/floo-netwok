import Qt.labs.platform as Platform

Platform.SystemTrayIcon {
    visible: true
    icon.source: "qrc:/images/icon"

    menu: Platform.Menu {
        Platform.Menu {
            title: qsTr("&About")

            Platform.MenuItem {
                text: qsTr("About me")

                onTriggered: Qt.openUrlExternally("https://github.com/TaipaXu")
            }

            Platform.MenuItem {
                text: qsTr("Project domain")

                onTriggered: Qt.openUrlExternally("https://github.com/TaipaXu/floo-network")
            }

            Platform.MenuItem {
                text: qsTr("Bug report")

                onTriggered: Qt.openUrlExternally("https://github.com/TaipaXu/floo-network/issues")
            }
        }

        Platform.MenuSeparator {}

        Platform.MenuItem {
            text: qsTr("Quit")

            onTriggered: Qt.quit()
        }
    }

    onActivated: {
        root.show()
        root.raise()
        root.requestActivate()
    }
}
