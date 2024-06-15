import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import network.receiver as Network

Rectangle {
    id: root
    color: "#3F4246"

    Flickable {
        id: flickable
        anchors.fill: parent
        flickableDirection: Flickable.VerticalFlick
        contentHeight: content.height
        clip: true
        topMargin: 0

        ColumnLayout {
            id: content
            width: parent.width

            Repeater {
                model: ReceiveManager.receivers
                delegate: Receiver {
                    required property Network.Receiver modelData

                    receiver: modelData
                    Layout.preferredWidth: parent.width - 16
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredHeight: 50
                }
            }
        }
    }

    Connections {
        target: ReceiveManager

        function onReceiversChanged() {
            console.log("onReceiversChanged")
        }
    }
}
