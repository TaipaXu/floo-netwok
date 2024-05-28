import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.platform
import model.channel as Model
import model.myFile as Model
import model.file as Model
import network.server as Network
import utils

Rectangle {
    id: root
    color: "#3F4246"

    required property Model.Channel channel

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        RowLayout {
            Layout.fillWidth: false
            Layout.preferredWidth: parent.width - 20
            Layout.margins: 10
            spacing: 10

            ColumnLayout {
                Label {
                    text: root.channel.name
                }
                Label {
                    text: root.channel.address + ":" + root.channel.port
                }
            }

            Item {
                Layout.fillWidth: true
            }

            Button {
                id: startButton
                text: qsTr("Start")
                enabled: network.tcpStatus === Network.Server.Unconnected

                onClicked: {
                    const result = network.start(root.channel.address, root.channel.port);
                    if (!result) {
                        messageDialog.open();
                    } else if (enableWeb.checked) {
                        network.startWs(root.channel.address);
                    }
                }
            }

            Button {
                id: stopButton
                text: qsTr("Stop")
                enabled: network.tcpStatus === Network.Server.Connected && network.wsStatus !== Network.Server.Connecting

                onClicked: {
                    network.stop();

                    if (network.wsStatus === Network.Server.Connected) {
                        network.stopWs();
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: false
            Layout.preferredWidth: parent.width - 20
            Layout.margins: 10
            spacing: 3

            Label {
                text: qsTr("Web")
            }

            Switch {
                id: enableWeb
                enabled: network.wsStatus !== Network.Server.Connecting
                checked: root.channel.webEnabled

                onCheckedChanged: {
                    root.channel.webEnabled = checked;
                    if (checked && network.tcpStatus === Network.Server.Connected && network.wsStatus === Network.Server.Unconnected) {
                        network.startWs(root.channel.address);
                        return;
                    }
                    if (!checked && network.wsStatus === Network.Server.Connected) {
                        network.stopWs();
                    }
                }
            }

            Image {
                visible: network.wsStatus === Network.Server.Connected
                fillMode: Image.PreserveAspectFit
                smooth: true
                antialiasing: true
                Layout.preferredWidth: 18
                Layout.preferredHeight: 18
                source: "qrc:/images/copy"

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor

                    onClicked: () => {
                        utils.copyToClipboard(`http://${root.channel.address}:${root.channel.webServerPort}/web/index.html?wsPort=${root.channel.wsPort}`);
                    }
                }
            }

            Item {
                Layout.fillWidth: true
            }
        }

        Row {
            id: flowView
            width: parent.width
            Layout.fillWidth: true
            Layout.preferredWidth: parent.width - 20
            Layout.margins: {
                top: 0
                right: 10
                bottom: 0
                left: 10
            }
            spacing: 10

            Tag {
                text: qsTr("My Files")
                active: stackView.currentIndex == 0

                onClicked: {
                    stackView.currentIndex = 0;
                }
            }

            Repeater {
                model: network.connections

                Tag {
                    text: modelData.address
                    active: stackView.currentIndex == index + 1

                    onClicked: {
                        stackView.currentIndex = index + 1;
                    }
                }
            }
        }

        Rectangle {
            width: parent.width
            Layout.fillWidth: true
            Layout.fillHeight: true

            color: "#1E1E1E"

            StackLayout {
                id: stackView
                anchors.fill: parent
                currentIndex: 0

                Flickable {
                    id: flickable
                    flickableDirection: Flickable.VerticalFlick
                    contentHeight: content.height
                    topMargin: 0
                    clip: true

                    ColumnLayout {
                        id: content
                        width: parent.width
                        spacing: 10

                        Item { }

                        Repeater {
                            model: network.myFiles
                            delegate: MyFile {
                                required property Model.MyFile modelData

                                Layout.preferredWidth: parent.width - 16
                                Layout.alignment: Qt.AlignHCenter
                                file: modelData

                                onRemove: network.removeMyFile(modelData)
                            }
                        }
                    }
                }

                Repeater {
                    model: network.connections
                    delegate: Flickable {
                        flickableDirection: Flickable.VerticalFlick
                        contentHeight: content.height
                        topMargin: 0
                        clip: true

                        ColumnLayout {
                            id: content
                            width: parent.width
                            spacing: 10

                            Item { }

                            Repeater {
                                model: modelData.files
                                delegate: File {
                                    required property Model.File modelData

                                    Layout.preferredWidth: parent.width - 16
                                    Layout.alignment: Qt.AlignHCenter
                                    file: modelData

                                    onDownload: network.requestDownloadFile(modelData)
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    DropArea {
        id: dropArea
        anchors.fill: parent

        property bool dragging: false

        onEntered: (drag) => {
            if (drag.hasUrls) {
                drag.accepted = true;
            } else {
                drag.accepted = false;
            }
            dropArea.dragging = true;
        }
        onDropped: (drop) => {
            dropArea.dragging = false;
            network.addMyFiles(drop.urls);
        }

        Rectangle {
            anchors.fill: parent
            color: "black"
            opacity: 0.2
            visible: dropArea.dragging
        }
    }

    Network.Server {
        id: network

        onConnectionsChanged: {
            console.log("Server Channel Connections Changed");
            if (stackView.currentIndex !== 0) {
                if (network.connections.length === 0) {
                    stackView.currentIndex = 0;
                } else if (stackView.currentIndex < network.connections.length) {
                    stackView.currentIndex = network.connections.length;
                }
            }
        }

        onWsInfoChanged: function(wsPort, weServerPort) {
            console.log("Server Channel WS Info Changed");
            root.channel.wsPort = wsPort;
            root.channel.webServerPort = weServerPort;
        }
    }

    MessageDialog {
        id: messageDialog
        text: qsTr("Failed to start server channel!")
        buttons: MessageDialog.Ok
    }

    Utils {
        id: utils
    }

    Component.onDestruction: console.log("Server Channel Destruction Beginning!")
}
