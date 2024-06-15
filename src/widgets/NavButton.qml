import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    color: "transparent"

    property string unactiveImage
    property string activeImage
    property bool active: false
    property bool hover: false

    signal clicked
    signal contextMenuRequested

    Image {
        anchors.centerIn: parent
        width: parent.width - 10
        height: width
        fillMode: Image.PreserveAspectFit
        smooth: true
        antialiasing: true
        source: root.active || root.hover ? root.activeImage : root.unactiveImage
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor

        onEntered: hover = true

        onExited: hover = false

        onClicked: (mouse) => {
            if (mouse.button === Qt.LeftButton) {
                root.clicked();
            } else if (mouse.button === Qt.RightButton) {
                root.contextMenuRequested();
            }
        }
    }
}
