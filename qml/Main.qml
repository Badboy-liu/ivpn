import QtQuick 6.8
import QtQuick.Controls 6.3
import QtQuick.Layouts 6.3
import QtQuick.Window 6.4

ApplicationWindow {
    id: window
    width: 720
    height: 680
    visible: true
    title: "IVPN Tool"
    color: "#f0f2f5"

    property string currentNode: ""
    Connections {
        target: backend

        function onNodesUpdated(list) {
            listView.model = list
        }

        function onSpeedProgress(cur, total) {
            progress.value = total > 0 ? (cur / total * 100) : 0
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 16

        Label {
            text: "节点转换工具"
            font.pixelSize: 24
            font.bold: true
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
        }
        // ===== 订阅栏 =====
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            TextField {
                id: subUrl
                placeholderText: "输入订阅链接（支持 base64 / clash / sing-box）"
                Layout.fillWidth: true
                Layout.minimumHeight: 30
            }

            Button {
                text: "加载订阅"
                onClicked: backend.loadSubscription(subUrl.text)
            }

            Button {
                text: "测速"
                onClicked: backend.startSpeedTest()
            }
        }
        ProgressBar {
            id: progress
            Layout.fillWidth: true
            from: 0
            to: 100
        }

        // ===== 输入框 =====
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 140
            radius: 10
            color: "white"
            border.color: "#ccc"

            TextArea {
                id: input
                anchors.fill: parent
                anchors.margins: 10
                placeholderText: "粘贴节点（多行）"
                wrapMode: TextArea.Wrap
            }
        }

        // ===== 操作栏 =====
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            ComboBox {
                id: target
                Layout.fillWidth: true
                model: ["vmess","vless","trojan", "ss", "ssr","hy2","hysteria","tuic","wg"]
                Layout.minimumHeight: 30
            }

            Button {
                text: "转换"
                Layout.preferredWidth: 120

                onClicked: {
                    output.text = backend.convert(input.text, target.currentText)

                    var lines = output.text.split(/\r?\n/).filter(s => s.length > 0)
                    if (lines.length > 0) {
                        currentNode = lines[0]
                        qrImage.source = backend.genQr(lines[0]) + "&t=" + Date.now()
                    }

                    listView.model = lines
                }
            }
        }

        // ===== 主区域 =====
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 16

            // ===== 左：节点列表 =====
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: 10
                color: "white"
                border.color: "#ccc"

                ListView {
                    id: listView
                    anchors.fill: parent
                    anchors.margins: 6
                    spacing: 6
                    clip: true
                    model: []

                    delegate: Rectangle {
                        width: listView.width
                        height: textItem.paintedHeight + 12
                        radius: 6
                        color: ListView.isCurrentItem ? "#d0e6ff" : "white"
                        border.color: "#ddd"

                        Text {
                            id: textItem
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.margins: 6
                            text: modelData
                            wrapMode: Text.Wrap
                            font.pixelSize: 12
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                listView.currentIndex = index
                                currentNode = modelData
                                qrImage.source = backend.genQr(modelData) + "&t=" + Date.now()
                            }
                        }
                    }
                }
            }

            // ===== 右：二维码 =====
            Rectangle {
                Layout.preferredWidth: 240
                Layout.fillHeight: true
                radius: 10
                color: "white"
                border.color: "#ccc"

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 10

                    Image {
                        id: qrImage
                        Layout.preferredWidth: 200
                        Layout.preferredHeight: 200
                        fillMode: Image.PreserveAspectFit
                        source: ""
                    }

                    Label {
                        text: currentNode === "" ? "请选择节点" : "二维码"
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }
        }

        // 隐藏输出缓存
        TextArea {
            id: output
            visible: false
        }
    }
}