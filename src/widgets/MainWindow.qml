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

        onRequestCreateChannel: showCreateChannelDialog()
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

            onRequestCreateChannel: showCreateChannelDialog()
        }

        StackLayout {
            id: mainStack
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: getCurrentIndex()

            Home {
                Layout.fillWidth: true
                Layout.fillHeight: true

                onRequestCreateChannel: showCreateChannelDialog()
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

    function showCreateChannelDialog() {
        const component = Qt.createComponent("qrc:/widgets/CreateChannelDialog.qml");
        if (component.status === Component.Ready) {
            const dialog = component.createObject(root);
            dialog.accepted.connect(onAccepted);
            dialog.show();
        }
    }

    function onAccepted(name, address, port) {
        console.log("onAccepted", name, address, port);
    }
}
