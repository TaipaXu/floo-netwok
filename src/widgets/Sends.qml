import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import network.sender as Network

Rectangle {
    id: root
    color: "#3F4246"

    Flickable {
        id: flickable
        anchors.fill: parent
        flickableDirection: Flickable.VerticalFlick
        contentHeight: content.height
        topMargin: 0
        clip: true

        ColumnLayout {
            id: content
            width: parent.width

            Repeater {
                model: SendManager.senders
                delegate: Sender {
                    required property Network.Sender modelData

                    sender: modelData
                    Layout.preferredWidth: parent.width - 16
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredHeight: 50
                }
            }
        }
    }
}
