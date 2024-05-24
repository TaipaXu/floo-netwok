import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import network.receiver as Network

Rectangle {
    id: root
    color: "transparent"

    required property Network.Receiver receiver

    component InfoLabel: Label {
        Layout.preferredWidth: 100
        elide: Text.ElideLeft
    }

    RowLayout {
        anchors.fill: parent
        spacing: 2

        InfoLabel {
            text: receiver.name
        }

        InfoLabel {
            text: receiver.size
        }

        ProgressBar {
            Layout.fillWidth: true
            value: root.receiver.progress
        }

        InfoLabel {
            text: receiver.statusName
        }
    }
}
