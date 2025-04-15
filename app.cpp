#include "app.h"

using namespace std::chrono_literals;

bool App::init_regex()
{
    try
    {
        input_file_rx = std::regex(input_mask);
        return true;
    }
    catch (const std::regex_error& e)
    {
        std::stringstream ss;

        ss << "Поймано исключение во время инициализации регулярного выражения:\n\n";
        ss << e.what();

        qCritical("%ls", ss.str().c_str());
        emit wantToShowMessage(ss.str().c_str());
    }

    return false;
}

bool App::init_fs_path(const std::string str_path, std::filesystem::path& path)
{
    try
    {
        path = std::filesystem::canonical(str_path);
        return true;
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        std::stringstream ss;

        ss << "Поймано исключение во время конвертации \"";
        ss << str_path << "\" в std::filesystem::path:\n\n";
        ss << e.what();

        qCritical("%ls", ss.str().c_str());
        emit wantToShowMessage(ss.str().c_str());
    }

    return false;
}

bool App::get_first_available_file_num(std::filesystem::path& path)
{
    std::string file_extension, file_name = path.string();

    if (path.has_extension())
    {
        auto s = path.string();
        size_t idx = s.rfind(".");
        file_extension = s.substr(idx);
        file_name = s.substr(0, idx);
    }
    else
    {
        file_name = path.string();
    }

    for (int i = 1; i < 10'000; i++)
    {
        std::stringstream new_zero_padded_path;

        new_zero_padded_path << file_name << "-";
        new_zero_padded_path << std::setw(4) << std::setfill('0') << i;

        if (!file_extension.empty())
            new_zero_padded_path << file_extension;

        if (!std::filesystem::exists(new_zero_padded_path.str()))
        {
            path = new_zero_padded_path.str();
            return true;
        }
    }

    // Не будем же мы бесконечно его искать? 10к думаю достаточно.
    qWarning("Не смогли найти свободное число для \"%ls\" потому они все заняты (от 1 до 9,999)", path.c_str());
    return false;
}

void App::process_files()
{
    int total_files = 0;

    for (auto& path : std::filesystem::directory_iterator(fs_input_dir))
    {
        auto cur_path = path.path().filename().c_str();

        if (should_exit)
            break;

        if (path.is_directory())
        {
            qInfo("Пропускаем \"%ls\" потому что это директория", cur_path);
            continue;
        } else if (!does_file_match_input_mask(path.path().filename().string()))
        {
            qInfo("Пропускаем \"%ls\" потому что файл не подходит под маску", cur_path);
            continue;
        }

        total_files++;
    }

    int idx = 0;
    for (auto& path : std::filesystem::directory_iterator(fs_input_dir))
    {
        auto cur_path = path.path().c_str();

        if (should_exit)
            break;

        idx++;
        set_total_progress(static_cast<float>(idx) / total_files);

        if (path.is_directory() || !does_file_match_input_mask(path.path().filename().string()))
            continue;

        std::ifstream in_stream(path.path(), std::ios_base::binary);

        if (!in_stream.is_open())
        {
            qWarning("Пропускаем \"%ls\" потому что не смогли открыть входной файл", cur_path);
            continue;
        }

        std::filesystem::path out_path = fs_output_dir / path.path().filename();

        if (std::filesystem::exists(out_path) && existing_file_action == ExistingFileAction::ADD_COUNTER)
        {
            std::filesystem::path new_path = out_path;

            if (get_first_available_file_num(new_path))
            {
                out_path /= new_path;
            }
            else
            {
                qWarning("Пропускаем \"%ls\" потому что не смогли заменить имя файла.", cur_path);
                continue;
            }
        }

        std::ofstream out_stream(out_path, std::ios_base::binary);

        if (!out_stream.is_open())
        {
            qWarning("Пропускаем \"%ls\" потому что не смогли открыть выходной файл", cur_path);
            continue;
        }

        qDebug("Входной  файл: \"%ls\"", cur_path);
        qDebug("Выходной файл: \"%ls\"", out_path.c_str());

        bool ret = do_xor_on_file(in_stream, out_stream, path.file_size());

        if (!ret)
        {
            out_stream.close();
            std::filesystem::remove(out_path);
            qWarning("Не удалось выполнить операцию XOR над \"%ls\"", cur_path);
            continue;
        }

        if (ret && should_delete_input_files)
        {
            in_stream.close();
            std::filesystem::remove(path);
            qDebug("Удалили \"%ls\"", cur_path);
        }

        qDebug("Файл \"%ls\" успешно обработан.", out_path.c_str());
        qDebug("");
    }
}

bool App::does_file_match_input_mask(const std::string path)
{
    std::smatch match;

    if (input_mask.empty())
        return true;

    if (!std::regex_match(path, match, input_file_rx) || match.size() == 0)
    {
        // qDebug("match.size() = %lu", match.size());
        return false;
    }

    return true;
}

static uint64_t bswap(uint64_t val)
{
    uint64_t result = 0;

    asm ("bswap %0\n"
        : "=r" (result)
        : "r" (val));

    return result;
}

bool App::do_xor_on_file(std::ifstream& in, std::ofstream& out, size_t file_size)
{
    const size_t arr_size = 4 * 1024 * 1024; // 4 MB
    std::vector<char> buf(arr_size);

    size_t total_read = 0;
    while (!in.eof())
    {
        if (should_exit)
        {
            qInfo("Пользователь запросил остановку");
            return false;
        }

        in.read(buf.data(), arr_size);
        total_read += in.gcount();

        // qDebug("Прочитали %llu байт", in.gcount());

        // это нужно потому что hex_value хранится в little-endian
        // и если пользователь введет маску 0xDEADBEEF, она будет храниться
        // как 0xFEBEADDE что будет выдавать неправильные результаты.
        // данная строчка по сути ломает алгоритм на big-endian процессорах
        uint64_t swapped = bswap(hex_value);

        uint64_t* p = reinterpret_cast<uint64_t*>(buf.data());
        size_t i = 0;

        for (i = 0; i < in.gcount() / sizeof(uint64_t); i++)
            p[i] ^= swapped;

        char bytes_left = in.gcount() - (i * sizeof(uint64_t));

        if (bytes_left > 0)
        {
            // qDebug("У нас осталось %d байт для xor'а", bytes_left);

            char bits_to_shift = (sizeof(uint64_t) - bytes_left) * 8;
            p[i] ^= swapped << bswap(bits_to_shift);
        }

        out.write(buf.data(), in.gcount());
        out.flush();

        if (out.fail())
        {
            qWarning("Не смогли записать %llu байт", in.gcount());
            return false;
        }

        set_file_progress(static_cast<float>(total_read) / file_size);
    }

    qDebug("Суммарно обработано %llu байт", total_read);
    return true;
}

void App::do_work()
{
    if (!(init_fs_path(input_dir, fs_input_dir) && init_fs_path(output_dir, fs_output_dir) && init_regex()))
        return;

    process_files();
    qInfo("Файлы обработаны");
}

void App::work_thread()
{
    do_work();
    emit workThreadFinished();
}

void App::stop_work_thread()
{
    should_exit = true;

    if (thread.joinable())
        thread.join();

    should_exit = false;
    already_running = false;
}

void App::set_total_progress(float val)
{
    // qDebug("progress = %.2f", val);
    emit totalProgressChanged(val);
}

void App::set_file_progress(float val)
{
    // qDebug("progress = %.2f", val);
    emit fileProgressChanged(val);
}

App app = App();
