import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

ApplicationWindow {
    id: root

    property real scale: applicationScale
    property real unscaledWidth: 1080
    property real unscaledHeight: 1487

    x: 0
    y: 0
    width: unscaledWidth * scale
    height: unscaledHeight * scale

    background.anchors.topMargin: -218 * scale
    background.anchors.bottomMargin: -215 * scale

    SwipeView {
        id: swipeView
        anchors.centerIn: parent
        width: root.unscaledWidth
        height: root.unscaledHeight - root.footer.height / applicationScale
        scale: root.scale
        currentIndex: tabBar.currentIndex

        Page1 {
        }

        Page {
            Label {
                text: qsTr("Second page")
                anchors.centerIn: parent
            }
        }
    }

    footer: TabBar {
        id: tabBar
        scale: root.scale
        currentIndex: swipeView.currentIndex
        TabButton {
            text: qsTr("First")
        }
        TabButton {
            text: qsTr("Second")
        }
    }
}
