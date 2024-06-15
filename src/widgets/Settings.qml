import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.platform
import persistence.settings as Persistence

Flickable {
    flickableDirection: Flickable.VerticalFlick
    contentHeight: contentItem.childrenRect.height
    ScrollBar.vertical: ScrollBar {}

    Pane {
        width: parent.width

        ColumnLayout {
            width: parent.width
            spacing: 10

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Label {
                    width: 60
                    text: qsTr("Download Location")
                }

                Item {
                    Layout.fillWidth: true
                }

                Label {
                    text: settings.downloadPath
                }

                Item {
                    Layout.preferredWidth: 30
                }

                Button {
                    text: qsTr("Select")

                    onClicked: folderDialog.open()
                }
            }
        }
    }

    Persistence.Settings {
        id: settings
    }

    FolderDialog {
        id: folderDialog

        onAccepted: settings.downloadPath = folderDialog.folder
    }
}
