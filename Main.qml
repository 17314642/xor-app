import QtQuick
import QtQuick.Layouts
import QtQuick.Controls 2.15
import QtQuick.Dialogs

import "helpers.js" as Helpers

Window {
    id: window
    width: 700
    height: 480
    visible: true
    minimumHeight: 480
    minimumWidth: 700
    title: qsTr("XOR app")

    property double timerTriggerTime: 0

    MessageDialog {
        id: msg_box
        buttons: MessageDialog.Ok
    }

    GridLayout {
        visible: true
        anchors.fill: parent
        anchors.leftMargin: 50
        anchors.rightMargin: 50
        anchors.topMargin: 50
        anchors.bottomMargin: 50
        clip: true
        rowSpacing: 5
        columnSpacing: 5
        uniformCellWidths: false
        uniformCellHeights: false
        rows: 10
        columns: 3


        Text {
            id: _text4
            text: qsTr("Входная директория")
            font.pixelSize: 12
        }


        TextInput {
            id: input_dir
            width: 80
            height: 20
            text: qsTr("C:\\Users\\qemu\\Desktop\\инпуты")
            font.pixelSize: 12
            Layout.columnSpan: 1
            font.bold: false
            font.italic: false
            font.underline: false
            font.strikeout: false
            autoScroll: true
            clip: true
            cursorVisible: true
            Layout.fillWidth: true
        }

        Button {
            id: selectInputDirButton
            text: qsTr("Выбрать")
            highlighted: false
            Layout.columnSpan: 1
            onClicked: input_dir_folder_dialog.open()
        }

        FolderDialog {
            id: input_dir_folder_dialog
            onAccepted: {
                input_dir.text = App.convert_file_url(input_dir_folder_dialog.selectedFolder);
            }
        }

        Text {
            id: _text1
            text: qsTr("Выходная директория")
            font.pixelSize: 12
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
        }

        TextInput {
            id: output_dir
            width: 80
            height: 20
            text: qsTr("C:\\Users\\qemu\\Desktop\\outputs")
            font.pixelSize: 12
            Layout.columnSpan: 1
            clip: true
            cursorVisible: true
            Layout.fillWidth: true
        }

        Button {
            id: selectOutputDirButton
            text: qsTr("Выбрать")
            highlighted: false
            Layout.columnSpan: 1
            onClicked: output_dir_folder_dialog.open()
        }

        FolderDialog {
            id: output_dir_folder_dialog
            onAccepted: {
                output_dir.text = App.convert_file_url(output_dir_folder_dialog.selectedFolder);
            }
        }

        Text {
            id: _text
            text: qsTr("Входная маска:")
            font.pixelSize: 12
        }


        TextInput {
            id: input_mask
            width: 80
            height: 20
            text: qsTr(".*\\.bin")
            font.pixelSize: 12
            clip: true
            Layout.columnSpan: 2
            Layout.fillWidth: true
        }


        CheckBox {
            id: should_delete_input_files
            text: qsTr("Удалить входные файлы")
            checkState: Qt.Unchecked
            Layout.columnSpan: 1
            Layout.rowSpan: 1
        }




        Switch {
            id: timerSwitch
            text: qsTr("Запуск по таймеру (минуты):")
        }

        SpinBox {
            id: timerSpinBox
            value: 5
            stepSize: 1
            to: 300
            from: 1
            wheelEnabled: true
            wrap: true
            editable: true
        }

        Text {
            id: _text2
            text: qsTr("Действие при существующем файле")
            font.pixelSize: 12
            wrapMode: Text.WordWrap
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
        }






        ComboBox {
            id: existing_file_action
            Layout.columnSpan: 2
            Layout.fillWidth: true
            model: ["Добавить счётчик к имени файла", "Перезаписать"]
            currentIndex: 1
        }






        Text {
            id: _text3
            text: qsTr("Hex значение (макс. 8 байт)")
            font.pixelSize: 12
            wrapMode: Text.WordWrap
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
        }






        TextInput {
            id: hex_value
            width: 80
            height: 20
            text: qsTr("DEADBEEFDEADBEEF")
            inputMask: "hhhhhhhhhhhhhhhh"
            font.pixelSize: 12
            Layout.columnSpan: 2
            Layout.fillWidth: true
        }







        Button {
            id: runButton
            text: qsTr("Старт")
            highlighted: false
            flat: false
            Layout.columnSpan: 3
            Layout.fillWidth: true
            onClicked: Helpers.run_button_handler()
        }





        ProgressBar {
            id: totalProgressBar
            value: 0
            Layout.columnSpan: 3
            Layout.fillWidth: true
            indeterminate: false
            enabled: false
        }





        ProgressBar {
            id: fileProgressBar
            value: 0
            Layout.columnSpan: 3
            Layout.fillWidth: true
            indeterminate: false
            enabled: false
        }





        ScrollView {
            id: scrollView
            width: 200
            height: 200
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.columnSpan: 3

            TextArea {
                id: logArea
                width: parent.width
                height: parent.height
                readOnly: true
                placeholderText: qsTr("Здесь будут логи...")
            }
        }





        Timer {
            id: workTimer
            running: false
            repeat: true
            interval: 30000

            onTriggered: {
                console.log("workTimer");

                if (App.already_running) {
                    workTimer.stop();
                    statusTimer.stop();
                    timerStatus.text = "Статус таймера: ожидаем окончания текущей работы";
                } else {
                    Helpers.set_timer_trigger_time();
                    Helpers.start_work();
                }
            }
        }

        Timer {
            id: statusTimer
            running: false
            repeat: true
            interval: 1000

            onTriggered: {
                let left = (timerTriggerTime - new Date()) / 1000;

                let hours = Math.floor(left / 3600);
                let minutes = Math.floor((left - (hours * 3600)) / 60);
                let seconds = Math.floor(left - (hours * 3600) - (minutes * 60));

                hours = hours.toString().padStart(2, "0");
                minutes = minutes.toString().padStart(2, "0");
                seconds = seconds.toString().padStart(2, "0");

                timerStatus.text = `Статус таймера: до запуска осталось ${hours}:${minutes}:${seconds}`;
            }
        }



    }

    Text {
        id: timerStatus
        x: 0
        y: 463
        width: parent.width
        height: 20
        text: qsTr("Статус таймера: выключен")
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0
        font.pixelSize: 12
        wrapMode: Text.WordWrap
        maximumLineCount: 5
        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
    }

    Connections {
        target: App

        function onTotalProgressChanged(val) {
            // console.log(val);
            totalProgressBar.value = val;
        }

        function onFileProgressChanged(val) {
            // console.log(val);
            fileProgressBar.value = val;
        }

        function onWantToShowMessage(msg) {
            msg_box.text = msg;
            msg_box.open();
        }

        function onWorkThreadFinished() {
            Helpers.reset_controls(/*called_from_handler*/ true);
        }

        function onLogNewMessage(msg) {
            logArea.text += msg;
            logArea.cursorPosition = logArea.length - 1;
        }
    }


}
