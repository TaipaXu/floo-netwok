import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import model.file as Model

Rectangle {
    id: root
    height: 110
    color: "#3F4246"
    opacity: 0.8
    radius: 5

    required property Model.File file

    RowLayout {
        id: rowLayout
        anchors.fill: parent
        anchors.margins: 10

        ColumnLayout {
            spacing: 5

            Label {
                text: root.file.name
                font.pixelSize: 16
                Layout.fillWidth: true
                elide: Text.ElideLeft
            }

            Item {
                width: 1
                height: 5
            }

            Label {
                text: root.file.size
                font.pixelSize: 10
                Layout.fillWidth: true
                elide: Text.ElideLeft
            }
        }


        Item {
            Layout.fillWidth: true
        }

        Button {
            text: qsTr("Download")

            onClicked: {

            }
        }
    }
}
