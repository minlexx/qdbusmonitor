import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Controls 2.0

Window {
    visible: true
    width: 640
    height: 480
    title: qsTr("Hello World")

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
            enabled: !monitor.isRunning
            onClicked: {
                app.startOnSessionBus();
            }
        }

        Button {
            text: qsTr("Start on system bus")
            enabled: !monitor.isRunning
            onClicked: {
                app.startOnSystemBus();
            }
        }

        Button {
            text: qsTr("Stop monitor")
            enabled: monitor.isRunning
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
        delegate: ItemDelegate {
            implicitHeight: col1.height
            Column {
                id: col1
                Row {
                    height: txtSender.height
                    Text {
                        id: txtSender
                        text: model.sender
                    }
                    Text { text: "  =>  " }
                    Text { text: model.destination }
                }
            }
        }
    }
}
