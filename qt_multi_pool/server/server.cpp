#include "server.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QString>
#include <QDataStream>
#include <QCoreApplication>

#ifdef QT_DEBUG
#include <QDebug>
#include <QElapsedTimer>
#endif

StorageClientThread::StorageClientThread(qintptr socketDescriptor, IStorage *storage, QObject *parent)
    : QThread(parent), m_pSocketDescriptor(socketDescriptor), m_pStorage(storage), m_pOperation(-1)
{
    m_pReadCount = 0;
    m_pWriteCount = 0;
}

void StorageClientThread::run()
{
    if (!m_pStorage)
        return;

    qDebug() << this->currentThreadId() << "run";

    QTcpSocket tcpSocket;
    if (!tcpSocket.setSocketDescriptor(m_pSocketDescriptor)) {
        qDebug() << this->currentThreadId() << "error: setSocketDescriptor" << m_pSocketDescriptor;
        return;
    }

    QMetaObject::Connection connection = connect(&tcpSocket, &QTcpSocket::readyRead, [&]() {
        QDataStream in(&tcpSocket);

        qint64 bytesReady = tcpSocket.bytesAvailable();
        if (bytesReady < sizeof(int))
            return;
        if (m_pOperation < 0) {
            in >> m_pOperation;
        }

        switch (m_pOperation) {
        case IStorage::opRead: {
            if (bytesReady < sizeof(int) * 2)
                return;
            int index;
            in >> index;
            int result = m_pStorage->readItem(index);
            sendResult(&tcpSocket, result);

            m_pOperation = -1;
            m_pReadCount++;
            m_pWriteCount++;
        }
            break;
        case IStorage::opWrite: {
            if (bytesReady < sizeof(int) * 3)
                return;
            int item;
            int index;
            in >> index >> item;
            m_pStorage->writeItem(index, item);

            m_pOperation = -1;
            m_pWriteCount++;
        }
            break;
        default: {
            Q_ASSERT(false);
        }
            break;
        }
    });

    connect(&tcpSocket, SIGNAL(disconnected()), this, SLOT(quit()));

    exec();


    qDebug() << this->currentThreadId() << tcpSocket.errorString();
    qDebug() << this->currentThreadId() << "stop" << "read ops:" << m_pReadCount << "write ops:" << m_pWriteCount;
}

void StorageClientThread::sendResult(QTcpSocket *socket, int result)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << result;
    socket->write(block);
    socket->flush();
}

StorageServer::StorageServer(int port, QObject *parent) : QTcpServer(parent), m_pStorage(new ThreadSafeIntListStorage())
{
    if (!listen(QHostAddress::Any, port)) {
        close();
        qDebug() << errorString();
    }
    else
        qDebug() << "Server started";
}

StorageServer::~StorageServer()
{
    delete m_pStorage;
}

void StorageServer::incomingConnection(qintptr descriptor)
{
    StorageClientThread *thread = new StorageClientThread(descriptor, m_pStorage);
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), this, SLOT(printStatistics()));
    thread->start();
    qDebug() << "new client connection";
}

void StorageServer::printStatistics()
{
    m_pStorage->printStatistics();
}

ThreadSafeIntListStorage::ThreadSafeIntListStorage() : m_pLock()
{
#ifdef QT_DEBUG
    qDebug() << "Storage created" << m_readOpCount << m_writeOpCount;
#endif
}

ThreadSafeIntListStorage::~ThreadSafeIntListStorage()
{
#ifdef QT_DEBUG
    qDebug() << "Storage read operations:" << m_readOpCount << "write operations:" << m_writeOpCount++;
#endif
}

void ThreadSafeIntListStorage::writeItem(int index, const int &item)
{
#ifdef QT_DEBUG
    QElapsedTimer timer;
    timer.start();
    static quint64 totalWriteTime = 0;
#endif

    QWriteLocker locker(&m_pLock);

    if (index < 0 || index >= m_List.count())
        m_List.append(item);
    else
        m_List[index] = item;

#ifdef QT_DEBUG
    qint64 time = timer.nsecsElapsed();
    totalWriteTime += (quint64)time;
    m_maxWriteWaitTime = qMax((qint64)m_maxWriteWaitTime, time);
    if (!m_minWriteWaitTime)
        m_minWriteWaitTime = time;
    else
        m_minWriteWaitTime = qMin((qint64)m_minWriteWaitTime, time);
    m_writeOpCount++;
    m_avgWriteWaitTime = totalWriteTime / m_writeOpCount;
#endif
}

int ThreadSafeIntListStorage::readItem(int index)
{
#ifdef QT_DEBUG
    QElapsedTimer timer;
    timer.start();
    static quint64 totalReadTime = 0;
#endif

    QReadLocker locker(&m_pLock);

    int result = 0;
    if (index >= 0 && index < m_List.count())
        result = m_List.at(index);

#ifdef QT_DEBUG
    qint64 time = timer.nsecsElapsed();
    totalReadTime += (quint64)time;
    m_maxReadWaitTime = qMax((qint64)m_maxReadWaitTime, time);
    if (!m_minReadWaitTime)
        m_minReadWaitTime = time;
    else {
        m_minReadWaitTime = qMin((qint64)m_minReadWaitTime, time);
    }
    m_readOpCount++;
    m_avgReadWaitTime = totalReadTime / m_readOpCount;
#endif

    return result;
}

void ThreadSafeIntListStorage::printStatistics()
{
    qDebug() << "--------------> STATISTICS <--------------";    
    qDebug() << "Items count:" << m_List.count();
    QString itemsStr;
    foreach (auto item, m_List) {
        itemsStr += QString("%1; ").arg(item);
    }
    qDebug() << "Items:" << itemsStr;

#ifdef QT_DEBUG
    qDebug() << "Storage read operations:" << m_readOpCount << "write operations:" << m_writeOpCount++;
    qDebug() << "Read time(ns):";
    qDebug() << "max:" << m_maxReadWaitTime;
    qDebug() << "avg:" << m_avgReadWaitTime;
    qDebug() << "min:" << m_minReadWaitTime;
    qDebug() << "Write time(ns):";
    qDebug() << "max:" << m_maxWriteWaitTime;
    qDebug() << "avg:" << m_avgWriteWaitTime;
    qDebug() << "min:" << m_minWriteWaitTime;
#endif

    qDebug() << "------------------------------------------";
}
