#ifndef APP_H
#define APP_H

#include <QGuiApplication>
#include <QQmlProperty>
#include <QUrl>

#include <iostream>
#include <thread>
#include <regex>
#include <filesystem>
#include <fstream>

#if __cplusplus < 201703L
#error C++17 is not enabled!
#endif

enum ExistingFileAction {
    ADD_COUNTER,
    OVERWRITE
};

class App : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool already_running READ get_already_running CONSTANT)

private:
    std::thread thread;
    std::atomic<bool> should_exit{false}, already_running{false};
    std::string input_dir, output_dir, input_mask;
    uint64_t hex_value;
    bool should_delete_input_files = false;
    int existing_file_action = ExistingFileAction::ADD_COUNTER;

    void work_thread();
    void do_work();
    void process_files();
    void set_total_progress(float val);
    void set_file_progress(float val);
    bool init_fs_path(const std::string str_path, std::filesystem::path& path);
    bool init_regex();
    bool does_file_match_input_mask(const std::string path);
    bool get_first_available_file_num(std::filesystem::path& path);
    bool do_xor_on_file(std::ifstream &in, std::ofstream &out, size_t file_size);

    std::filesystem::path fs_input_dir, fs_output_dir;
    std::regex input_file_rx;

public:
    explicit App(QObject *parent = nullptr) : QObject{parent} {}

    ~App()
    {
        stop_work_thread();
    }

    bool get_already_running()
    {
        return already_running;
    }

    Q_INVOKABLE QString convert_file_url(const QString path)
    {
        return QUrl(path).toLocalFile();
    }

    Q_INVOKABLE void stop_work_thread();

    Q_INVOKABLE void start_work_thread(
        QString qs_input_dir, QString qs_output_dir, QString qs_input_mask,
        bool delete_input_files, int file_action, QString qs_hex_value
        ) {
        input_dir = qs_input_dir.toStdString();
        output_dir = qs_output_dir.toStdString();
        input_mask = qs_input_mask.toStdString();
        hex_value = std::stoull(qs_hex_value.toStdString(), 0, 16);
        should_delete_input_files = delete_input_files;
        existing_file_action = file_action;

        qDebug();
        qDebug("input_dir                   = %s", input_dir.c_str());
        qDebug("output_dir                  = %s", output_dir.c_str());
        qDebug("input_mask                  = %s", input_mask.c_str());
        qDebug("should_delete_input_files   = %s", should_delete_input_files ? "true" : "false");
        qDebug("existing_file_action        = %d", existing_file_action);
        qDebug("hex_value                   = 0x%08llX", hex_value);

        if (!already_running) {
            already_running = true;
            thread = std::thread(&App::work_thread, this);
        }
    }

signals:
    void totalProgressChanged(float val);
    void fileProgressChanged(float val);
    void wantToShowMessage(QString msg);
    void workThreadFinished();
    void logNewMessage(QString msg);
};

extern App app;

#endif // APP_H
