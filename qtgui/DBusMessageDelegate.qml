import QtQuick 2.0
import QtQuick.Controls 2.0

ItemDelegate {
    id: delegate
    implicitHeight: mainRow.height + 5
    // width: parent.width // set from parent
    // highlighted: ListView.isCurrentItem // set from parent

    signal showReply(int id)
    signal showRequest(int id)

    property int serial
    property int replySerial
    property string typeString: ""

    property bool isSignal:       typeString === "signal"
    property bool isError:        typeString === "error"
    property bool isMethodCall:   typeString === "method call"
    property bool isMethodReturn: typeString === "method return"

    property string senderAddress
    property string senderExe
    property int    senderPid
    property var    senderNames: undefined // JavaScript array (QStringList)
    property string senderNamesStr
    property string senderNamesStrShort

    property string destinationAddress
    property string destinationExe
    property int    destinationPid
    property var    destinationNames: undefined // JavaScript array (QStringList)
    property string destinationNamesStr
    property string destinationNamesStrShort

    property string msgPath: ""
    property string msgInterface: ""
    property string msgMember: ""

    property int longNamesTruncateLimit: 60
    property int innerRectMargin: 5

    property color colorBorderSignal: "green"
    property color colorBorderError: "red"
    property color colorBorderNormal: "black"
    property color colorAddress: "gray"
    property color colorInterface: "#6c3109"
    property color colorMember: "#c06121"
    property color colorPath: "black"
    property color colorExe: "#0b2a8f"

    function truncateString(s, limit) {
        var ret = s;
        if (ret.length > limit) {
            ret = ret.substring(0, limit-3) + '...';
        }
        return ret;
    }

    Component.onCompleted: {
        // ^^ create a string like: "org.freedesktop.Notifications,org.kde.StatusNotifierHost-4344,
        //                           org.kde.plasmashell,com.canonical.Unity"
        if (senderNames !== undefined) {
            senderNamesStr = senderNames.join(", ");
            senderNamesStrShort = truncateString(senderNamesStr, longNamesTruncateLimit);
        }
        if (destinationNames !== undefined) {
            destinationNamesStr = destinationNames.join(", ");
            destinationNamesStrShort = truncateString(destinationNamesStr, longNamesTruncateLimit);
        }
    }

    Row {
        id: mainRow
        height: senderRect.height > destRect.height ? senderRect.height : destRect.height
        spacing: 5

        Rectangle {
            id: senderRect
            radius: isSignal ? innerRectMargin*2 : innerRectMargin
            border.width: 2
            border.color: isSignal ? colorBorderSignal : (isError ? colorBorderError : colorBorderNormal)
            implicitWidth: innerCol1.width + innerRectMargin*2
            implicitHeight: innerCol1.height + innerRectMargin*2


            Column {
                id: innerCol1
                spacing: 2
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: innerRectMargin

                Row {
                    Text {
                        text: "[" + senderAddress + "] "
                        color: colorAddress
                    }
                    Text {
                        text: senderNamesStrShort
                        font.bold: true
                    }
                }

                Text {
                    text: senderExe + " (pid: " + senderPid + ")"
                    color: colorExe
                    font.italic: true
                }

                Text {
                    text: "path: " + msgPath
                    // visible: isSignal
                    visible: msgPath !== ""
                    color: colorPath
                }

                Row {
                    visible: isSignal
                    Text {
                        text: msgInterface + "."
                        color: colorInterface
                        font.bold: true
                    }
                    Text {
                        text: msgMember + "()"
                        color: colorMember
                        font.bold: true
                    }
                }
            }
        }

        Column {
            spacing: 5
            Text {
                text: isMethodCall ? "  =>  " : (isMethodReturn ? " <= " : "")
                visible: !isSignal
            }
            Text {
                text: isMethodCall ? serial : replySerial
                visible: !isSignal
            }
        }

        Rectangle {
            id: destRect
            radius: innerRectMargin
            visible: !isSignal
            border.width: 2
            border.color: isError ? colorBorderError : colorBorderNormal
            implicitWidth: innerCol2.width + innerRectMargin*2
            implicitHeight: innerCol2.height + innerRectMargin*2

            Column {
                id: innerCol2
                spacing: 2
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: innerRectMargin

                Row {
                    Text {
                        text: "[" + destinationAddress + "] "
                        color: colorAddress
                    }
                    Text {
                        text: destinationNamesStrShort
                        font.bold: true
                    }
                }

                Text {
                    text: destinationExe + " (pid: " + destinationPid + ")"
                    color: colorExe
                    font.italic: true
                }

                Text {
                    text: "path: " + msgPath
                    visible: msgPath !== ""
                    color: colorPath
                }

                Row {
                    visible: !isSignal && !isMethodReturn
                    Text {
                        text: msgInterface + "."
                        color: colorInterface
                        font.bold: true
                    }
                    Text {
                        text: msgMember + "()"
                        color: colorMember
                        font.bold: true
                    }
                }
            }
        }

        Button {
            id: btnToReply
            visible: isMethodCall
            text: qsTr("To reply")
            onClicked: showReply(serial)
        }

        Button {
            id: btnToCall
            visible: isMethodReturn
            text: qsTr("To request")
            onClicked: showRequest(replySerial)
        }

    }

    onClicked: {
        // console.log("clicked:" + index);
    }
}
