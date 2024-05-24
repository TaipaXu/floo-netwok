import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import network.sender as Network

Rectangle {
    id: root
    color: "transparent"

    required property Network.Sender sender

    component InfoLabel: Label {
        Layout.preferredWidth: 100
        elide: Text.ElideLeft
    }

    RowLayout {
        anchors.fill: parent
        spacing: 2

        InfoLabel {
            text: sender.name
        }

        InfoLabel {
            text: sender.size
        }

        ProgressBar {
            Layout.fillWidth: true
            value: root.sender.progress
        }

        InfoLabel {
            text: sender.statusName
        }
    }
}
