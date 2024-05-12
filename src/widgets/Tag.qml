import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Label {
    id: root
    color: active || hover ? "red" : "white"

    property bool active: false
    property bool hover: false

    signal clicked

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        hoverEnabled: true

        onEntered: {
            root.hover = true
        }

        onExited: {
            root.hover = false
        }

        onClicked: root.clicked()
    }
}
