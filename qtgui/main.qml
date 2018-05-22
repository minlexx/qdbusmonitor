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

        delegate: ItemDelegate {
            id: delegate
            height: col1.height
            width: parent.width
            highlighted: ListView.isCurrentItem

            property bool isSignal: model.typeString === "signal"
            property bool isError:  model.typeString === "error"

            property string senderNamesStr: model.senderNames.join(",")
            property string destinationNamesStr: model.destinationNames.join(",")
            // ^^ creates a string like: "org.freedesktop.Notifications,org.kde.StatusNotifierHost-4344,
            //                            org.kde.plasmashell,com.canonical.Unity"

            Column {
                id: col1
                Row {
                    height: txtType.height
                    spacing: 5
                    Text {
                        id: txtType
                        text: "(" + model.typeString + ") "
                    }
                    Text {
                        text: "[" + model.senderAddress + "]"
                    }
                    Text {
                        text: model.senderExe !== "" ? model.senderExe : senderNamesStr
                    }
                    Text {
                        text: "  =>  "
                        visible: !delegate.isSignal
                    }
                    Text {
                        text: "[" + model.destinationAddress + "]"
                        visible: !delegate.isSignal
                    }
                    Text {
                        text: model.destinationExe !== "" ? model.destinationExe : destinationNamesStr
                        visible: (!delegate.isSignal) && (model.destinationName !== model.destinationAddress)
                    }
                    Text { text: model.path }
                    Text { text: model.interface }
                    Text { text: model.member }
                }
            }

            onClicked: {
                messagesView.currentIndex = index;
            }
        }

        ScrollIndicator.vertical: ScrollIndicator { }
    }
}
