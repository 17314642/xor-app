#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#ifdef _WIN32
#include <windows.h>
#endif

#include "app.h"

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

    std::cerr << ss.str();
    emit app.logNewMessage(ss.str().c_str());
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
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
