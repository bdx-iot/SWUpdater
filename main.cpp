#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "swupdater.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);

    SWUpdater *swupdater = new SWUpdater();
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("swupdater", swupdater);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
