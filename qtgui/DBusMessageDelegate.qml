import QtQuick 2.0
import QtQuick.Controls 2.0

ItemDelegate {
    id: delegate
    implicitHeight: row1.height
    // width: parent.width // set from parent
    // highlighted: ListView.isCurrentItem // set from parent

    property bool isSignal: false
    property bool isError: false

    property string typeString: ""

    property string senderAddress: ""
    property string senderExe: ""
    property var    senderNames: undefined // JavaScript array (QStringList)
    property string senderNamesStr: ""

    property string destinationAddress: ""
    property string destinationExe: ""
    property var    destinationNames: undefined // JavaScript array (QStringList)
    property string destinationNamesStr: ""

    property string msgPath: ""
    property string msgInterface: ""
    property string msgMember: ""

    Component.onCompleted: {
        // ^^ create a string like: "org.freedesktop.Notifications,org.kde.StatusNotifierHost-4344,
        //                           org.kde.plasmashell,com.canonical.Unity"
        if (senderNames !== undefined) {
            senderNamesStr = senderNames.join(",");
        }
        if (destinationNames !== undefined) {
            destinationNamesStr = destinationNames.join(",");
        }
    }

    Row {
        id: row1
        height: senderRect.height
        spacing: 5

        Rectangle {
            id: senderRect
            radius: isSignal ? 10 : 0
            border.width: 2
            border.color: isSignal ? "green" : (isError ? "red" : "black")
            implicitWidth: innerCol1.width
            implicitHeight: innerCol1.height


            Column {
                id: innerCol1
                spacing: 5
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: 5

                Text {
                    id: txtType
                    text: "(" + typeString + ") "
                }
                Text {
                    text: "[" + senderAddress + "]"
                }
                Text {
                    text: senderExe !== "" ? senderExe : senderNamesStr
                }
            }
        }

        Text {
            text: "  =>  "
            visible: !isSignal
        }

        Rectangle {
            id: destRect
            visible: !isSignal
            border.width: 2
            border.color: isError ? "red" : "black"
            implicitWidth: innerCol2.width
            implicitHeight: innerCol2.height

            Column {
                id: innerCol2
                spacing: 5
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: 5

                Text {
                    text: "[" + destinationAddress + "]"
                }
                Text {
                    text: destinationExe !== "" ? destinationExe : destinationNamesStr
                    visible: destinationNamesStr !== destinationAddress
                }
            }
        }

        Text { text: msgPath }
        Text { text: msgInterface }
        Text { text: msgMember }
    }
}
