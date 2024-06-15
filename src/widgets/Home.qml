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

    component RawButton: Button {
        flat: true
        font.pixelSize: 30

        HoverHandler {
            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
            cursorShape: Qt.PointingHandCursor
        }
    }

    Column {
        anchors.centerIn: parent

        Label {
            text: "Welcome to the <strong>Floo Network</strong>!"
            font.pixelSize: 40
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter

            RawButton {
                text: "Create a channel"

                onClicked: root.requestCreateChannel()

            }

            RawButton {
                text: "Join a channel"

                onClicked: root.requestJoinChannel()
            }
        }

        Item {
            width: parent.width
            height: 80
        }
    }
}
