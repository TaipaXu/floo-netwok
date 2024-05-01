import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    title: qsTr("Floo Network")
    width: 1200
    height: 800
    minimumWidth: 700
    minimumHeight: 350
    visible: true

    menuBar: MainWindowMenuBar {
        id: menuBar

        onRequestCreateChannel: {

        }
        onRequestJoinChannel: {

        }

        onRequestShowUploader: {

        }
        onRequestShowDownloader: {

        }
    }

    RowLayout {
        anchors.fill: parent
        Layout.fillWidth: true
        Layout.fillHeight: true

        Navbar {
            id: navbar
            width: 40
            Layout.fillHeight: true

            onCurrentTypeChanged: {
                console.log("currentTypeChanged", navbar.currentType);
            }
        }
    }
}
