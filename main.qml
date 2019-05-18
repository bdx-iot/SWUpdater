import QtQuick 2.2
import QtQuick.Controls 1.2

ApplicationWindow {
    visible: true
    title: "SWUpdater"
    width: 580
    height: 480

    TabView {
        anchors.fill: parent
        anchors.margins: 8
        Tab {
            id: controlPage
            title: "File"
            SWupdater { }
        }
    }
}
