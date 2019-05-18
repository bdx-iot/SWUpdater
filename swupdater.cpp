#include "swupdater.h"
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QHttpMultiPart>

SWUpdater::SWUpdater()
{
    m_manager =  new QNetworkAccessManager;
    connect(m_manager, &QNetworkAccessManager::finished, this, &SWUpdater::onUploadFinished);
    connect(&m_webSocket, &QWebSocket::connected, this, &SWUpdater::onConnected);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &SWUpdater::onDisconnected);
    connect(this, &SWUpdater::urlChanged, this, &SWUpdater::onUrlChanged);
    connect(this, &SWUpdater::uploadReady, this, &SWUpdater::onUpload);
    connect(this, &SWUpdater::updateFinished, this, &SWUpdater::onUpdateFinished);
    //m_url = "192.168.1.28";
    //m_file = "/home/jun/Documents/swupdate-workspace/update-image-imx7s-warp.swu";
    //Q_EMIT urlChanged();
}

void SWUpdater::onConnected()
{
    m_connected = true;
    qDebug() << "Connected to " << m_url;
    connect(&m_webSocket, &QWebSocket::textMessageReceived, this, &SWUpdater::onMessage);
}

void SWUpdater::onUrlChanged()
{
    if (m_onUpdate)
        return;

    m_websocket_url = m_url;

    m_url = QString("%1//%2:8080/%3").arg("http:", m_url, "upload");
    qDebug() << m_url;

    m_websocket_url = QString("%1//%2:8080/%3").arg("ws:", m_websocket_url, "ws");
    qDebug() << m_websocket_url;
    m_onUpdate = true;
    Q_EMIT uploadReady();

}

void SWUpdater::onUpload()
{
    QFile *file = new QFile(m_file);

    if(!file->open(QIODevice::ReadOnly)){
        qDebug() << "Open file failed ! " << "Abord update";
        return;
    }else{
        qDebug() << "Open file success !";
    }
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QNetworkRequest url = QNetworkRequest(QUrl(m_url));
    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("update-image-imx7s-warp/swu"));
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"update-image-imx7s-warp\""));
    filePart.setBodyDevice(file);
    file->setParent(multiPart);
    multiPart->append(filePart);
    m_webSocket.open(m_websocket_url);
    QNetworkReply *reply = m_manager->post(url, multiPart);

    multiPart->setParent(reply);
    connect(reply, SIGNAL(finished()), this, SLOT(onUploadFinished()));
    connect(reply, SIGNAL(uploadProgress(qint64, qint64)), this, SLOT(onUploadProgress(qint64, qint64)));
}

void SWUpdater::onMessage(const QString &message)
{
    QJsonParseError error;
    QString webMessage;
    QJsonDocument jsonMessage = QJsonDocument::fromJson(message.toUtf8(), &error);

    // Parse json
    if (error.error) {
        qWarning() << "Failed to parse text message as JSON object:" << message
                   << "Error is:" << error.errorString();
        return;
    } else if (!jsonMessage.isObject()) {
        qWarning() << "Received JSON message that is not an object: " << message;
        return;
    }

    // Get json object
    QJsonObject json = jsonMessage.object();
    if(json.contains("type") && json["type"].isString()){
        webMessage = json["type"].toString();
        if (webMessage != "message") {
            // TODO
        }

    }

    if (json.contains("text") && json["text"].isString()) {
        webMessage = json["text"].toString();
        qDebug() << webMessage;

        if(webMessage == "SWUPDATE successful !")
            Q_EMIT updateFinished();
    }
}

void SWUpdater::onUpdateFinished()
{
    m_onUpdate = false;
    m_webSocket.close();
}

void SWUpdater::onUploadFinished(QNetworkReply *reply)
{
    Q_UNUSED(reply);
    qDebug () << "Software image uploaded successfully. Wait for installation to be finished";

}

void SWUpdater::onUploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    double downloadProgress = static_cast<double>(bytesSent)/static_cast<double>(bytesTotal);
    qDebug() << "Uploading " << bytesSent  << "/" << bytesTotal << "%" << downloadProgress*100.0;
    //qDebug() << "---------Uploaded--------------" << bytesSent<< "of" <<bytesTotal;
}
