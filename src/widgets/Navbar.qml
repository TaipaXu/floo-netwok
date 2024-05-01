import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: "#333333"

    enum Type {
        Home,
        Channel,
        Upload,
        Download,
        Settings
    }

    readonly property int spacingValue: 4
    property int currentType: Navbar.Type.Home

    signal requestShowHome
    signal requestCreateChannel
    signal requestJoinChannel
    signal requestShowUploader
    signal requestShowDownloader

    ColumnLayout {
        id: column
        anchors.fill: parent
        spacing: spacingValue

        NavButton {
            width: parent.width
            height: width
            unactiveImage: "qrc:/images/home"
            activeImage: "qrc:/images/home-active"
            active: root.currentType === Navbar.Type.Home

            onClicked: root.currentType = Navbar.Type.Home
        }

        NavButton {
            width: parent.width
            height: width
            unactiveImage: "qrc:/images/create"
            activeImage: "qrc:/images/create-active"

            onClicked: createOrJoinMenu.popup()
        }

        ScrollView {
            id: scrollView
            Layout.fillHeight: true
            width: parent.width

            Column {
                anchors.fill: parent
                spacing: spacingValue
            }
        }

        NavButton {
            width: parent.width
            height: width
            unactiveImage: "qrc:/images/upload"
            activeImage: "qrc:/images/upload-active"
            active: root.currentType === Navbar.Type.Upload

            onClicked: root.currentType = Navbar.Type.Upload
        }

        NavButton {
            width: parent.width
            height: width
            unactiveImage: "qrc:/images/download"
            activeImage: "qrc:/images/download-active"
            active: root.currentType === Navbar.Type.Download

            onClicked: root.currentType = Navbar.Type.Download
        }

        NavButton {
            width: parent.width
            height: width
            unactiveImage: "qrc:/images/settings"
            activeImage: "qrc:/images/settings-active"
        }
    }

    Menu {
        id: createOrJoinMenu

        MenuItem {
            text: qsTr("Create a channel")

            onTriggered: root.requestCreateChannel()
        }

        MenuItem {
            text: qsTr("Join a channel");

            onTriggered: root.requestJoinChannel()
        }
    }
}
