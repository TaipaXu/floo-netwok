import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    width: 300
    height: 300
    color: "transparent"

    signal requestCreateChannel
    signal requestJoinChannel

    Column {
        anchors.centerIn: parent

        Label {
            text: "Welcome to the <strong>Floo Network</strong>!"
            font.pixelSize: 40
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter

            Button {
                text: "Create a channel"
                flat: true
                font.pixelSize: 30

                onClicked: root.requestCreateChannel()

                HoverHandler {
                    acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
                    cursorShape: Qt.PointingHandCursor
                }
            }

            Button {
                text: "Join a channel"
                flat: true
                font.pixelSize: 30
                onClicked: root.requestJoinChannel()

                HoverHandler {
                    acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
                    cursorShape: Qt.PointingHandCursor
                }
            }
        }

        Item {
            width: parent.width
            height: 80
        }
    }
}
