import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.qmlmodels
import model.channel as Model
import utils

ApplicationWindow {
    id: root
    title: qsTr("Floo Network")
    width: 1200
    height: 800
    minimumWidth: 700
    minimumHeight: 350
    visible: true

    ListModel {
        id: channelsModel
    }

    menuBar: MainWindowMenuBar {
        id: menuBar

        onRequestCreateChannel: root.showCreateChannelDialog()
        onRequestJoinChannel: root.showJoinChannelDialog()

        onRequestShowSettings: {
            if (navbar.currentType != Navbar.Type.Settings) {
                navbar.currentType = Navbar.Type.Settings;
            }
        }
    }

    RowLayout {
        anchors.fill: parent

        Navbar {
            id: navbar
            width: 40
            Layout.fillHeight: true
            channels: getChannels()

            onRequestCreateChannel: root.showCreateChannelDialog()
            onRequestJoinChannel: root.showJoinChannelDialog()

            onRequestCloseChannel: (index) => {
                const channel = channelsModel.get(index).channel;
                channelsModel.remove(index);
                channel.destroy();

                if (navbar.channels.length === 0) {
                    navbar.currentType = Navbar.Type.Home;
                } else if (navbar.currentChannelIndex >= navbar.channels.length) {
                    navbar.currentChannelIndex = navbar.channels.length - 1;
                }
            }

            function getChannels() {
                const channels = [];
                for (let i = 0; i < channelsModel.count; i++) {
                    channels.push(channelsModel.get(i).channel);
                }
                return channels;
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

                onRequestCreateChannel: root.showCreateChannelDialog()
                onRequestJoinChannel: root.showJoinChannelDialog()
            }

            StackLayout {
                id: stackView
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: navbar.currentChannelIndex

                Repeater {
                    model: channelsModel
                    DelegateChooser {
                        role: "type"

                        DelegateChoice {
                            roleValue: Model.Channel.Server
                            ItemDelegate {
                                required property var modelData

                                ServerChannel {
                                    width: parent.width
                                    height: parent.height
                                    channel: modelData.channel
                                }
                            }
                        }

                        DelegateChoice {
                            roleValue: Model.Channel.Client
                            ItemDelegate {
                                required property var modelData

                                ClientChannel {
                                    width: parent.width
                                    height: parent.height
                                    channel: modelData.channel
                                }
                            }
                        }
                    }
                }
            }

            Sends {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            Receives {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            Settings {
                Layout.fillWidth: true
                Layout.fillHeight: true
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

    Shortcut {
        sequences: [StandardKey.FullScreen]

        onActivated: {
            if (root.visibility === Window.FullScreen) {
                root.visibility = Window.Windowed;
            } else {
                root.visibility = Window.FullScreen;
            }
        }
    }

    Shortcut {
        sequence: StandardKey.Close

        onActivated: root.close()
    }

    Shortcut {
        sequence: StandardKey.Quit

        onActivated: Qt.quit()
    }

    Tray { }

    Utils {
        id: utils
    }

    function showCreateChannelDialog() {
        const component = Qt.createComponent("qrc:/widgets/CreateChannelDialog.qml");
        if (component.status === Component.Ready) {
            const dialog = component.createObject(root);
            dialog.accepted.connect(onCreateChannelAccepted);
            dialog.show();
        }
    }

    function onCreateChannelAccepted(name, address, port, webEnabled) {
        const channel = Qt.createQmlObject(`
            import model.channel

            Channel {
                type: Channel.Server
                name: "${name}"
                address: "${address}"
                port: ${port}
                webEnabled: ${webEnabled}
            }
        `, root);
        channelsModel.append({
            type: Model.Channel.Server,
            channel: channel
        });
        navbar.currentChannelIndex = channelsModel.count - 1;
        navbar.currentType = Navbar.Type.Channel;
    }

    function showJoinChannelDialog() {
        const component = Qt.createComponent("qrc:/widgets/JoinChannelDialog.qml");
        if (component.status === Component.Ready) {
            const dialog = component.createObject(root);
            dialog.accepted.connect(onJoinChannelAccepted);
            dialog.show();
        }
    }

    function onJoinChannelAccepted(address, port) {
        const channel = Qt.createQmlObject(`
            import model.channel

            Channel {
                type: Channel.Client
                address: "${address}"
                port: ${port}
            }
        `, root);
        channelsModel.append({
            type: Model.Channel.Client,
            channel: channel
        });
        navbar.currentChannelIndex = channelsModel.count - 1;
        navbar.currentType = Navbar.Type.Channel;
    }
}
