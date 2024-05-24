import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import network.sender as Network

Rectangle {
    id: root
    color: "transparent"

    required property Network.Sender sender
    property int bytesReceived: 1
    property int totalBytes : 1

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
            from: 0
            to: totalBytes
            value: bytesReceived
        }

        InfoLabel {
            text: sender.statusName
        }
    }

    Connections {
        target: sender

        function onProgressChanged(bytesReceived, totalBytes) {
            console.log("onProgressChanged: ", bytesReceived, totalBytes);
            root.bytesReceived = bytesReceived;
            root.totalBytes = totalBytes;
        }

        function onStatusChanged(status) {
            console.log("onStatusChanged: ", status);
            if (status === Network.Sender.Status.Finished) {
                root.bytesReceived = 1;
                root.totalBytes = 1;
            }
        }
    }
}
