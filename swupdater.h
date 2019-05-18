#ifndef SWUPDATER_H
#define SWUPDATER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QtWebSockets/QWebSocket>

class SWUpdater : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString qUrl MEMBER m_url NOTIFY urlChanged)
public:
    explicit SWUpdater();


Q_SIGNALS:
    void onDisconnected();
    void urlChanged();
    void uploadReady();
    void updateFinished();

public Q_SLOTS:
    void onConnected();
    void onUrlChanged();
    void onUpload();
    void onMessage(const QString &message);
    void onUpdateFinished();
    void onUploadFinished(QNetworkReply *reply);
    void onUploadProgress(qint64 bytesSent, qint64 bytesTotal);

private:
    QWebSocket m_webSocket;
    QString m_url;
    QString m_websocket_url;
    bool m_connected = false;
    bool m_onUpdate = false;
    QString m_file;
    QNetworkAccessManager *m_manager = nullptr;
};

#endif // SWUPDATER_H
