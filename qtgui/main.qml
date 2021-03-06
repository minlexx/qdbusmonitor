import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Controls 2.0

Window {
    visible: true
    width: 1152
    height: 864
    title: qsTr("QDBusMonitor")

    Flow {
        id: flow1
        anchors {
            left: parent.left
            top: parent.top
            right: parent.right
            margins: 10
        }

        spacing: 10

        Button {
            text: qsTr("Start on session bus")
            enabled: !monitor.isMonitorActive
            onClicked: {
                app.startOnSessionBus();
            }
        }

        Button {
            text: qsTr("Start on system bus")
            enabled: !monitor.isMonitorActive
            onClicked: {
                app.startOnSystemBus();
            }
        }

        Button {
            text: qsTr("Stop monitor")
            enabled: monitor.isMonitorActive
            onClicked: {
                app.stopMonitor();
            }
        }

        Button {
            text: qsTr("Clear")
            onClicked: {
                app.clearLog();
            }
        }

        CheckBox {
            id: cbAutoScroll
            checked: true
            text: qsTr("Autoscroll")
        }

        Button {
            text: qsTr("Quit")
            onClicked: {
                console.log("Quit from QML!");
                Qt.quit();
            }
        }
    }

    ListView {
        id: messagesView
        anchors {
            top: flow1.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            margins: 5
        }
        model: app.messagesModel
        interactive: true
        clip: true

        delegate: DBusMessageDelegate {
            id: delegate
            width: parent.width
            highlighted: ListView.isCurrentItem

            serial: model.serial
            replySerial: model.replySerial
            typeString: model.typeString

            senderAddress: model.senderAddress
            senderExe: model.senderExe
            senderPid: model.senderPid
            senderNames: model.senderNames

            destinationAddress: model.destinationAddress
            destinationExe: model.destinationExe
            destinationPid: model.destinationPid
            destinationNames: model.destinationNames

            msgPath: model.path
            msgInterface: model.interface
            msgMember: model.member

            onClicked: {
                messagesView.currentIndex = index;
            }

            onShowReply: {
                cbAutoScroll.checked = false;  // disable autoscroll
                var idx = app.messagesModel.findReplySerial(id);
                messagesView.positionViewAtIndex(idx, ListView.Center);
                messagesView.currentIndex = idx;
            }

            onShowRequest: {
                cbAutoScroll.checked = false;  // disable autoscroll
                var idx = app.messagesModel.findSerial(id);
                messagesView.positionViewAtIndex(idx, ListView.Center);
                messagesView.currentIndex = idx;
            }
        }

        ScrollBar.vertical: ScrollBar { }
    }

    Connections {
        target: app
        onAutoScroll: {
            if (cbAutoScroll.checked) {
                messagesView.positionViewAtEnd();
            }
        }
    }
}
