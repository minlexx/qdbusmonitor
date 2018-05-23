import QtQuick 2.0
import QtQuick.Controls 2.0

ItemDelegate {
    id: delegate
    implicitHeight: col1.height
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

    Column {
        id: col1
        Row {
            height: txtType.height
            spacing: 5
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
            Text {
                text: "  =>  "
                visible: !isSignal
            }
            Text {
                text: "[" + destinationAddress + "]"
                visible: !isSignal
            }
            Text {
                text: destinationExe !== "" ? destinationExe : destinationNamesStr
                visible: (!isSignal) && (destinationNamesStr !== destinationAddress)
            }
            Text { text: msgPath }
            Text { text: msgInterface }
            Text { text: msgMember }
        }
    }
}
