#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "app.h"

#ifdef _WIN32
#include <windows.h>

bool NEED_CONVERT_TO_CP1251 = false;

std::string utf8_to_cp1251(const std::string& str)
{
    uint32_t len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    std::vector<wchar_t> wstr(len);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wstr.data(), len);

    len = WideCharToMultiByte(1251, 0, wstr.data(), -1, NULL, 0, NULL, NULL);
    std::vector<char> cp1251(len);
    WideCharToMultiByte(1251, 0, wstr.data(), -1, cp1251.data(), len, NULL, NULL);

    return std::string(cp1251.data());
}
#endif // _WIN32

std::string enum_to_str(QtMsgType type)
{
    switch (type)
    {
    case QtDebugMsg:
        return "DEBUG";

    case QtWarningMsg:
        return "WARNING";

    case QtCriticalMsg:
        return "CRITICAL";

    case QtFatalMsg:
        return "FATAL";

    case QtInfoMsg:
        return "INFO";
    }

    return "";
}

void log_handler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    std::stringstream ss;

    ss << enum_to_str(type) << ": " << msg.toStdString() << std::endl;

#ifdef _WIN32
    if (NEED_CONVERT_TO_CP1251)
        std::cerr << utf8_to_cp1251(ss.str());
    else
#endif // _WIN32
        std::cerr << ss.str();

    emit app.logNewMessage(ss.str().c_str());
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
    // Вывод информации если программа запущена из cmd.exe или powershell.exe
    if (AttachConsole(ATTACH_PARENT_PROCESS))
    {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }

    // В Qt Creator'е НЕ РАБОТАЕТ (всегда выдает 0)
    uint32_t cp = GetConsoleOutputCP();

    if (cp > 0 && cp != CP_UTF8)
        SetConsoleOutputCP(CP_UTF8);
    else
        // Будет всё равно выдавать кракозябры если у пользователя кодировка
        // отличная от CP-1251
        NEED_CONVERT_TO_CP1251 = true;
#endif

    qInstallMessageHandler(log_handler);

    QGuiApplication gui_app(argc, argv);
    QQmlApplicationEngine engine;

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &gui_app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("test-app", "Main");

    QQmlContext *context = engine.rootContext();
    context->setContextProperty("App", &app);

    return gui_app.exec();
}
