import QtQuick 2.9
import QtQuick.Window 2.2
import QtQuick.Controls 2.2

Window {
    visible: true
    width: 640
    height: 480
    title: qsTr("Hello World")

    Button {
        anchors.centerIn: parent
        text: qsTr("Quit")
        onClicked: {
            console.log("Hey, need to quit!");
            Qt.quit();
        }
    }
}
