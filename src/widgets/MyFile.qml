import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import model.myFile as Model

Rectangle {
    id: root
    height: 110
    color: "#3F4246"
    opacity: 0.8
    radius: 5

    required property Model.MyFile file

    signal remove

    RowLayout {
        id: rowLayout
        anchors.fill: parent
        anchors.margins: 10

        ColumnLayout {
            spacing: 5

            Label {
                Layout.fillWidth: true
                text: root.file.name
                font.pixelSize: 16
                elide: Text.ElideLeft
            }

            Item {
                height: 5
            }

            Label {
                Layout.fillWidth: true
                text: root.file.size
                font.pixelSize: 10
                elide: Text.ElideLeft
            }

            LinkLabel {
                Layout.fillWidth: true
                text: root.file.path
                font.pixelSize: 12
                elide: Text.ElideLeft
                path: root.file.path
            }
        }


        Item {
            Layout.fillWidth: true
        }

        Button {
            text: qsTr("Remove")

            onClicked: root.remove()
        }
    }
}
