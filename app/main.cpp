#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QScreen>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    /* This is just for scaling down for testing UI on desktop */
    if ((QGuiApplication::primaryScreen()->orientation() == Qt::PortraitOrientation && QGuiApplication::primaryScreen()->availableSize().width() < 1487) ||
        (QGuiApplication::primaryScreen()->orientation() == Qt::LandscapeOrientation && QGuiApplication::primaryScreen()->availableSize().height() < 1487))
    {
        engine.rootContext()->setContextProperty("applicationScale", 0.5);
    }
    else
        engine.rootContext()->setContextProperty("applicationScale", 1);

    engine.load(QUrl(QLatin1String("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
