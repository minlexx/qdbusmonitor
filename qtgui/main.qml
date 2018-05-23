import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Controls 2.0

Window {
    visible: true
    width: 640
    height: 480
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
        }
        model: app.messagesModel
        interactive: true
        clip: true

        delegate: DBusMessageDelegate {
            id: delegate
            width: parent.width
            highlighted: ListView.isCurrentItem

            typeString: model.typeString
            isSignal: model.typeString === "signal"
            isError:  model.typeString === "error"

            senderAddress: model.senderAddress
            senderExe: model.senderExe
            senderNames: model.senderNames

            destinationAddress: model.destinationAddress
            destinationExe: model.destinationExe
            destinationNames: model.destinationNames

            msgPath: model.path
            msgInterface: model.interface
            msgMember: model.member

            onClicked: {
                messagesView.currentIndex = index;
            }
        }

        ScrollIndicator.vertical: ScrollIndicator { }
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
