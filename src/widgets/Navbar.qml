import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import model.channel 1.0 as Model

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
    property int currentChannelIndex: -1
    property int currentType: Navbar.Type.Home
    property list<Model.Channel> channels

    signal requestShowHome
    signal requestCreateChannel
    signal requestJoinChannel
    signal requestCloseChannel(int index)
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

                Repeater {
                    model: root.channels

                    delegate: NavButton {
                        width: parent.width
                        height: width
                        unactiveImage: modelData.type === Model.Channel.Server ? "qrc:/images/server" : "qrc:/images/client"
                        activeImage: modelData.type === Model.Channel.Server ?  "qrc:/images/server-active" : "qrc:/images/client-active"
                        active: root.currentType === Navbar.Type.Channel && root.currentChannelIndex === index

                        onClicked: {
                            root.currentChannelIndex = index;
                            root.currentType = Navbar.Type.Channel;
                        }

                        onContextMenuRequested: {
                            closeMenu.closeIndex = index;
                            closeMenu.popup();
                        }
                    }
                }
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
            text: qsTr("Join a channel")

            onTriggered: root.requestJoinChannel()
        }
    }

    Menu {
        id: closeMenu
        property int closeIndex: -1

        MenuItem {
            text: qsTr("Close this channel")

            onTriggered: root.requestCloseChannel(closeMenu.closeIndex)
        }
    }
}
