#ifndef CLIENT_H
#define CLIENT_H

#include <QThread>
#include <QTcpSocket>
#include "../common/common.h"

class RemoteClientThread : public QThread
{
    Q_OBJECT
public:
    RemoteClientThread(QString host, int port, IStorage::Operation operation, QObject *parent)
        : QThread(parent), m_pHost(host), m_pPort(port), m_pStop(false), m_pOperation(operation) {}
    void run() override;
    void stop() { m_pStop = true; }

protected:
    IStorage::Operation m_pOperation;

private:
    bool m_pStop;
    QString m_pHost;
    int m_pPort;
};

#endif // CLIENT_H
