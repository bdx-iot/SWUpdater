#include "swupdater.h"
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
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
}

SWUpdater::~SWUpdater()
{
    if (!m_file) {
        delete m_file;
        m_file = nullptr;
    }

    if (!m_manager) {
        delete m_manager;
        m_manager = nullptr;
    }
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
    qDebug() << m_pathImage;
    m_swuImage = extractImageName(m_pathImage);
    qDebug() << m_swuImage;
    m_onUpdate = true;
    Q_EMIT uploadReady();

}

void SWUpdater::onUpload()
{
    QString errMsg;
    m_pathImage.remove("file://");
    m_file = new QFile(m_pathImage);

    if (m_file->exists()){
        if(!m_file->open(QIODevice::ReadOnly)){
            errMsg = m_file->errorString();
            qDebug() << "Open file failed ! " << "Abord update" << "reason: " << errMsg;
            m_onUpdate = false;
            return;
        }else{
            qDebug() << "Open file success !";
        }
    }else {
        qDebug() << "No such file or directory, Abord update";
        return;
    }

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QNetworkRequest url = QNetworkRequest(QUrl(m_url));
    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant( m_swuImage + "/swu"));
    QString dispoHeader = QString("form-data; name=\"\%1\"").arg(m_swuImage );
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(dispoHeader));
    filePart.setBodyDevice(m_file);
    m_file->setParent(multiPart);
    multiPart->append(filePart);
    m_webSocket.open(m_websocket_url);
    QNetworkReply *reply = m_manager->post(url, multiPart);

    multiPart->setParent(reply);
    connect(reply, &QNetworkReply::uploadProgress, this, &SWUpdater::onUploadProgress);
}

void SWUpdater::onMessage(const QString &message)
{
    qint32 rc;
    QJsonParseError str_error;
    QString webMessage;
    QJsonDocument jsonMessage = QJsonDocument::fromJson(message.toUtf8(), &str_error);

    // Parse json
    if (str_error.error) {
        qWarning() << "Failed to parse text message as JSON object:" << message
                   << "Error is:" << str_error.errorString();
        return;
    } else if (!jsonMessage.isObject()) {
        qWarning() << "Received JSON message that is not an object: " << message;
        return;
    }

    // Get json object
    QJsonObject json = jsonMessage.object();
    if(json.contains("type") && json["type"].isString()) {
        webMessage = json["type"].toString();
        rc = webMessage.compare("message");
        if (rc < 0) {
            // TODO
        }

    }

    if (json.contains("text") && json["text"].isString()) {
        webMessage = json["text"].toString();
        qDebug() << webMessage;

        // Test string
        rc = webMessage.compare(STRING_SUCCESS, Qt::CaseInsensitive);
        if (rc < 0) {
            qDebug() << webMessage;
        } else {
            Q_EMIT onUpdateFinished();
        }

    }
}

void SWUpdater::onUpdateFinished()
{
    m_onUpdate = false;
    m_webSocket.close();
    qDebug() << "SWupdater finished update";
}

void SWUpdater::onUploadFinished()
{
    qDebug () << "Software image uploaded successfully. Wait for installation to be finished";
    if(m_file->isOpen()) {
        qDebug() << "close file";
        m_file->close();
    }
}

void SWUpdater::onUploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    double downloadProgress = static_cast<double>(bytesSent)/static_cast<double>(bytesTotal);
    qDebug() << "Uploading " << bytesSent  << "/" << bytesTotal << " - " << downloadProgress * 100.0 << "%";
}

QString SWUpdater::extractImageName(QString &path)
{
    QString url;
    QRegExp rgx("[/]");
    QStringList list = path.split(rgx, QString::SkipEmptyParts);

    foreach(QString str, list)
    {
        if (str.endsWith(".swu")) {
            url = str.split(".swu")[0];
            return url;
        }
    }

    return "";
}
