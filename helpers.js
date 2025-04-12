function run_button_handler() {
    if (runButton.checked) {
        workTimer.stop();
        statusTimer.stop();
        timerStatus.text = "Статус таймера: выключен";

        reset_controls();
    } else {
        runButton.checked = true;
        runButton.text = "Стоп";

        if (timerSwitch.checked) {
            workTimer.interval = timerSpinBox.value * 60 * 1000;
            set_timer_trigger_time();
            workTimer.start();
            statusTimer.start();
        }

        start_work();
    }
}

function start_work() {
    totalProgressBar.enabled = true;
    totalProgressBar.value = 0;

    fileProgressBar.enabled = true;
    fileProgressBar.value = 0;

    App.start_work_thread(
        input_dir.text,
        output_dir.text,
        input_mask.text,
        should_delete_input_files.checked,
        existing_file_action.currentIndex,
        hex_value.text
    );
}

function set_timer_trigger_time() {
    let t = new Date();
    t.setMinutes(t.getMinutes() + timerSpinBox.value);
    timerTriggerTime = t;
}

function reset_controls(calledFromHandler = false) {
    App.stop_work_thread();

    totalProgressBar.enabled = false;
    totalProgressBar.value = 0;

    fileProgressBar.enabled = false;
    fileProgressBar.value = 0;

    if (!calledFromHandler || !timerSwitch.checked) {
        runButton.checked = false;
        runButton.text = "Старт";
    } else if (calledFromHandler && runButton.checked && timerSwitch.checked && !workTimer.running) {
        // Если таймер сработал во время работы, то мы дожидаемся окончания работы
        // и тут же запускаемся снова
        set_timer_trigger_time();
        workTimer.start();
        statusTimer.start();
        start_work();
    }
}
