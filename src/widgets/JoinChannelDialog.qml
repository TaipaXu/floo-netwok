import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    width: 400
    height: 200
    title: qsTr("Join a channel")
    modality: Qt.WindowModal
    visible: true

    readonly property int defaultPort: 1024

    signal accepted(string address, int port)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 8

        TextField {
            id: channelAddress
            Layout.fillWidth: true
            placeholderText: qsTr("Channel address")
            focus: true

            Keys.onReturnPressed: channelPort.focus = true
        }


        TextField {
            id: channelPort
            Layout.fillWidth: true
            placeholderText: qsTr("Channel port")
            validator: IntValidator { bottom: 1024; top: 65535 }
            text: root.defaultPort.toString()

            Keys.onReturnPressed: root.submit()
        }

        Item {
            Layout.fillHeight: true
        }

        RowLayout {
            Item {
                Layout.fillWidth: true
            }

            Button {
                id: cancelButton
                text: qsTr("Cancel")

                onClicked: root.cancel()
            }

            Button {
                id: createChannelButton
                text: qsTr("OK")

                onClicked: root.submit()
            }
        }
    }

    Shortcut {
        sequence: StandardKey.Cancel

        onActivated: root.cancel()
    }

    onClosing: root.destroy

    function submit() {
        if (channelAddress.length > 0 && channelPort.length > 0) {
            root.accepted(channelAddress.text, parseInt(channelPort.text));
            root.close();
        }
    }

    function cancel() {
        root.close();
    }
}
