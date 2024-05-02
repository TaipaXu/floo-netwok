import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import utils

ApplicationWindow {
    id: root
    width: 400
    height: 200
    title: qsTr("Create a channel")
    modality: Qt.WindowModal
    visible: true

    readonly property int defaultPort: 1024

    signal accepted(string name, string address, int port)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 8

        TextField {
            id: channelName
            Layout.fillWidth: true
            placeholderText: qsTr("Channel name")
            text: qsTr("New channel")
            focus: true

            Keys.onReturnPressed: {
                channelAddress.focus = true;
            }
        }

        ComboBox {
            id: channelAddress
            Layout.fillWidth: true
            model: utils.getLocalAddresses()

            Keys.onReturnPressed: {
                channelPort.focus = true;
            }
        }


        TextField {
            id: channelPort
            Layout.fillWidth: true
            placeholderText: qsTr("Channel port")
            validator: IntValidator { bottom: 1024; top: 65535 }
            text: root.defaultPort.toString()

            Keys.onReturnPressed: submit()
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

                onClicked: cancel()
            }

            Button {
                id: createChannelButton
                text: qsTr("OK")

                onClicked: submit()
            }
        }
    }

    Shortcut {
        sequence: StandardKey.Cancel

        onActivated: cancel()
    }

    Utils {
        id: utils
    }

    function submit() {
        if (channelName.length > 0 && channelAddress.currentText.length > 0 && channelPort.length > 0) {
            root.accepted(channelName.text, channelAddress.currentText, parseInt(channelPort.text));
            root.close();
            root.destroy();
        }
    }

    function cancel() {
        root.close();
        root.destroy();
    }
}
