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

        StackLayout {
            id: mainStack
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: getCurrentIndex()

            Home {
                Layout.fillWidth: true
                Layout.fillHeight: true

                onRequestCreateChannel: {

                }
                onRequestJoinChannel: {

                }
            }

            function getCurrentIndex() {
                switch (navbar.currentType) {
                    case Navbar.Type.Home:
                        return 0;
                    case Navbar.Type.Channel:
                        return 1;
                    case Navbar.Type.Upload:
                        return 2;
                    case Navbar.Type.Download:
                        return 3;
                    case Navbar.Type.Settings:
                        return 4;
                }
            }
        }
    }

    Tray { }
}
