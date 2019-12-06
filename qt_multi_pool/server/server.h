#ifndef SERVER_H
#define SERVER_H
#include <QObject>
#include <QList>
#include <QThread>
#include <QTcpServer>
#include <QTcpSocket>
#include <QReadWriteLock>
#include "../common/common.h"

class ThreadSafeIntListStorage : public IStorage
{
public:
    ThreadSafeIntListStorage();
    ~ThreadSafeIntListStorage();
    void writeItem(int index, const int & item) override;
    int readItem(int index) override;
    void printStatistics() override;

private:
    QList<int> m_List;
    QReadWriteLock m_pLock;

#ifdef QT_DEBUG
    QAtomicInt m_readOpCount = 0;
    QAtomicInt m_writeOpCount = 0;
    QAtomicInt m_maxReadWaitTime = 0;
    QAtomicInt m_avgReadWaitTime = 0;
    QAtomicInt m_minReadWaitTime = 0;
    QAtomicInt m_maxWriteWaitTime = 0;
    QAtomicInt m_avgWriteWaitTime = 0;
    QAtomicInt m_minWriteWaitTime = 0;
#endif
};

class StorageClientThread : public QThread
{
    Q_OBJECT
public:
    StorageClientThread(qintptr socketDescriptor, IStorage *storage, QObject *parent = nullptr);

protected:
    void run() override;

signals:
    void error(QTcpSocket::SocketError socketError);

private:
    qintptr m_pSocketDescriptor;
    IStorage *m_pStorage;
    int m_pOperation;
    quint64 m_pReadCount;
    quint64 m_pWriteCount;

    void sendResult(QTcpSocket *socket, int result);
};

class StorageServer : public QTcpServer
{
    Q_OBJECT
public:
    StorageServer(int port, QObject *parent = 0);
    ~StorageServer();

protected:
    void incomingConnection(qintptr descriptor);

private:
    QStringList fortunes;
    IStorage* m_pStorage;

private slots:
    void printStatistics();
};

#endif // SERVER_H
