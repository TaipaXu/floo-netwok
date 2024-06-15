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

        Nav {
            unactiveImage: "qrc:/images/home"
            activeImage: "qrc:/images/home-active"
            active: root.currentType === Navbar.Type.Home

            onClicked: root.currentType = Navbar.Type.Home
        }

        Nav {
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

                    delegate: Nav {
                        active: root.currentType === Navbar.Type.Channel && root.currentChannelIndex === index

                        onClicked: {
                            root.currentChannelIndex = index;
                            root.currentType = Navbar.Type.Channel;
                        }

                        onContextMenuRequested: {
                            closeMenu.closeIndex = index;
                            closeMenu.popup();
                        }

                        Component.onCompleted: {
                            if (modelData.type === Model.Channel.Server) {
                                unactiveImage = "qrc:/images/server";
                                activeImage = "qrc:/images/server-active";
                            } else {
                                unactiveImage = "qrc:/images/client";
                                activeImage = "qrc:/images/client-active";
                            }
                        }
                    }
                }
            }
        }

        Nav {
            unactiveImage: "qrc:/images/upload"
            activeImage: "qrc:/images/upload-active"
            active: root.currentType === Navbar.Type.Upload

            onClicked: root.currentType = Navbar.Type.Upload
        }

        Nav {
            unactiveImage: "qrc:/images/download"
            activeImage: "qrc:/images/download-active"
            active: root.currentType === Navbar.Type.Download

            onClicked: root.currentType = Navbar.Type.Download
        }

        Nav {
            unactiveImage: "qrc:/images/settings"
            activeImage: "qrc:/images/settings-active"
            active: root.currentType === Navbar.Type.Settings

            onClicked: root.currentType = Navbar.Type.Settings
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

    component Nav: NavButton {
        width: parent.width
        height: width
    }
}
