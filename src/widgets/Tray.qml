import Qt.labs.platform as Platform

Platform.SystemTrayIcon {
    visible: true
    icon.source: "qrc:/images/icon"

    menu: Platform.Menu {
        Platform.Menu {
            title: qsTr("&About")

            Platform.MenuItem {
                text: qsTr("About me")

                onTriggered: utils.openMyGithubPage()
            }

            Platform.MenuItem {
                text: qsTr("Project domain")

                onTriggered: utils.openThisProjectPage()
            }

            Platform.MenuItem {
                text: qsTr("Bug report")

                onTriggered: utils.openThisProjectIssuesPage()
            }
        }

        Platform.MenuSeparator {}

        Platform.MenuItem {
            text: qsTr("Quit")

            onTriggered: Qt.quit()
        }
    }

    onActivated: {
        root.show();
        root.raise();
        root.requestActivate();
    }
}
