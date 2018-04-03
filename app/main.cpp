#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QScreen>
#include <QDebug>
#include <QQuickWindow>
#include <QCommandLineParser>
#include <QUrlQuery>
#include <libhomescreen.hpp>
#include <qlibwindowmanager.h>

int main(int argc, char *argv[])
{
    QString myname = QString("Hello QML");
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    QQmlContext *context = engine.rootContext();

    /* This is just for scaling down for testing UI on desktop */
    if ((QGuiApplication::primaryScreen()->orientation() == Qt::PortraitOrientation && QGuiApplication::primaryScreen()->availableSize().width() < 1487) ||
        (QGuiApplication::primaryScreen()->orientation() == Qt::LandscapeOrientation && QGuiApplication::primaryScreen()->availableSize().height() < 1487))
    {
        context->setContextProperty("applicationScale", 0.5);
    }
    else
        context->setContextProperty("applicationScale", 1);

    QCommandLineParser parser;
    parser.addPositionalArgument("port", app.translate("main", "port for binding"));
    parser.addPositionalArgument("secret", app.translate("main", "secret for binding"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);
    QStringList positionalArguments = parser.positionalArguments();

    if (positionalArguments.length() == 2) {
        int port = positionalArguments.takeFirst().toInt();
        QString secret = positionalArguments.takeFirst();
        QUrl bindingAddress;
        bindingAddress.setScheme(QStringLiteral("ws"));
        bindingAddress.setHost(QStringLiteral("localhost"));
        bindingAddress.setPort(port);
        bindingAddress.setPath(QStringLiteral("/api"));
        QUrlQuery query;
        query.addQueryItem(QStringLiteral("token"), secret);
        bindingAddress.setQuery(query);
        context->setContextProperty(QStringLiteral("bindingAddress"), bindingAddress);
        std::string token = secret.toStdString();
        QLibWindowmanager* qwm = new QLibWindowmanager();

        // WindowManager initialization
        if(qwm->init(port,secret) != 0){
            exit(EXIT_FAILURE);
        }
        // Request a surface as described in layers.json windowmanager’s file
        if (qwm->requestSurface(myname) != 0) {
            exit(EXIT_FAILURE);
        }
        //  Create an event handler against an event type. Here a lambda is called when SyncDraw event occurs
        qwm->set_event_handler(QLibWindowmanager::Event_SyncDraw, [qwm, myname](json_object *object) {
            fprintf(stderr, "Surface got syncDraw!\n");
            qwm->endDraw(myname);
        });

        LibHomeScreen* hs = new LibHomeScreen();

        // HomeScreen
        hs->init(port, token.c_str());
        // Set the event handler for Event_TapShortcut which will be delivered when the user click the short cut icon.
        hs->set_event_handler(LibHomeScreen::Event_TapShortcut, [qwm, myname](json_object *object){
            json_object *appnameJ = nullptr;
            if(json_object_object_get_ex(object, "application_name", &appnameJ))
            {
                const char *appname = json_object_get_string(appnameJ);
                if(myname == appname)
                {
                    qDebug("Surface %s got tapShortcut\n", appname);
                    qwm->activateSurface(myname);
                }
            }
        });

        engine.load(QUrl(QLatin1String("qrc:/main.qml")));
        QObject *root = engine.rootObjects().first();
        QQuickWindow *window = qobject_cast<QQuickWindow *>(root);
        QObject::connect(window, SIGNAL(frameSwapped()), qwm, SLOT(slotActivateSurface()
        ));
    }

    return app.exec();
}
