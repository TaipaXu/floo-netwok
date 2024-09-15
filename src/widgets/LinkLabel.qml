import QtQuick
import QtQuick.Controls

Label {
    id: root

    required property string path

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor

        onEntered: font.underline = true

        onExited: font.underline = false

        onClicked: utils.openFileDirectory(path)
    }
}
