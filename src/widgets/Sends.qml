import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.qmlmodels
import network.sender as Network

Rectangle {
    id: root
    color: "#3F4246"

    property var items: []

    HorizontalHeaderView {
        id: horizontalHeader
        anchors.left: tableView.left
        anchors.top: parent.top
        syncView: tableView
        clip: true

        delegate: Rectangle {
            color: "transparent"

            Label {
                text: [qsTr("Name"), qsTr("Size"), qsTr("Progress"), qsTr("Status")][index]
                anchors.centerIn: parent
            }
        }
    }

    TableView {
        id: tableView
        anchors.left: parent.left
        anchors.top: horizontalHeader.bottom
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        clip: true


        columnWidthProvider: (column) => {
            return tableView.width / tableView.model.columnCount;
        }

        model: TableModel {
            TableModelColumn { display: "name" }
            TableModelColumn { display: "size" }
            TableModelColumn { display: "progress" }
            TableModelColumn { display: "status" }

            rows: root.items
        }

        delegate: DelegateChooser {
            DelegateChoice {
                column: 0

                delegate: Cell {
                    label: display
                }
            }

            DelegateChoice {
                column: 1

                delegate: Cell {
                    label: display
                }
            }

            DelegateChoice {
                column: 2

                delegate: Rectangle {
                    color: "transparent"

                    RowLayout {
                        width: parent.width
                        spacing: 4

                        ProgressBar {
                            Layout.fillWidth: true
                            value: display
                        }

                        Label {
                            Layout.preferredWidth: 30
                            text: (display === 1 ? 100 : (display * 100).toFixed(1)) + "%"
                        }
                    }
                }
            }

            DelegateChoice {
                column: 3

                delegate: Cell {
                    label: display
                }
            }
        }
    }

    Connections {
        target: SendManager

        function onSendersChanged() {
            for (const item of SendManager.senders) {
                const found = root.items.find((element) => element.id === item.id);
                if (found === undefined) {
                    root.items.push({
                        "id": item.id,
                        "name": item.name,
                        "size": item.size,
                        "progress": item.progress,
                        "status": item.statusName
                    });
                    tableView.model.rows = root.items;

                    item.nameChanged.connect(() => {
                        const found = root.items.find((element) => element.id === item.id);
                        if (found !== undefined) {
                            found.name = item.name;
                            tableView.model.rows = root.items;
                        }
                    });
                    item.sizeChanged.connect(() => {
                        const found = root.items.find((element) => element.id === item.id);
                        if (found !== undefined) {
                            found.size = item.size;
                            tableView.model.rows = root.items;
                        }
                    });
                    item.progressChanged.connect(() => {
                        const found = root.items.find((element) => element.id === item.id);
                        if (found !== undefined) {
                            found.progress = item.progress;
                            tableView.model.rows = root.items;
                        }
                    });
                    item.statusChanged.connect(() => {
                        const found = root.items.find((element) => element.id === item.id);
                        if (found !== undefined) {
                            found.status = item.statusName;
                            tableView.model.rows = root.items;
                        }
                    });
                }
            }
        }
    }

    component Cell: Rectangle {
        color: "transparent"

        required property string label

        Label {
            text: label
            anchors.centerIn: parent
        }
    }
}
