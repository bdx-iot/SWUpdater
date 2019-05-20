import QtQuick 2.12
import QtQuick.Controls 1.2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.12
import QtQuick.Window 2.12

Item {
    width: 580
    height: 400
    SystemPalette { id: palette }
    clip: true

    FileDialog {
        id: fileDialog
        visible: false
        nameFilters: [ "SWUpdate Image (*.swu)"]
        selectedNameFilter: "SWUpdate Image (*.swu)"
        sidebarVisible: true
        onAccepted:  swupdater.qFile = fileUrl
    }

    ScrollView {
        id: scrollView
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            bottom: bottomBar.top
            leftMargin: 12
        }
        ColumnLayout {
            spacing: 8
            Item { Layout.preferredHeight: 4 } // padding
            Label {
                font.bold: true
                text: "SWUpdater Properties:"
            }
            TextField {
                id: tfPort
                width: 15
                text: "Port SWUpdate"
                font.family: "Helvetica"
                font.pointSize: 10
                focus: true
            }
            TextField {
                id: tffile
                width: 15
                text: fileDialog.fileUrl != "" ? fileDialog.fileUrl : qsTr("File")
                font.family: "Helvetica"
                font.pointSize: 10
                focus: true
                visible: fileDialog.fileUrl != "" ? true : false
            }

        }
    }

    ProgressBar {
        indeterminate: true
        visible: false
        anchors {
            centerIn: parent.Center
        }
        width: 2
        height: 2
    }
    Rectangle {
        id: bottomBar
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        height: buttonRow.height * 1.2
        color: Qt.darker(palette.window, 1.1)
        border.color: Qt.darker(palette.window, 1.3)
        Row {
            id: buttonRow
            spacing: 6
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 12
            height: implicitHeight
            width: parent.width
            Button {
                text: "Open"
                anchors.verticalCenter: parent.verticalCenter
                onClicked: fileDialog.open()
            }

            Button {
                id: btnvalid
                text: "Valider"
                onClicked: {
                    swupdater.qUrl = tfPort.text
                }
            }
        }
    }
}
