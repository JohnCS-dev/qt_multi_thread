#include "client.h"
#include <QDataStream>

void RemoteClientThread::run()
{
    const int MaxItemsCount = 10;
    const int MaxItemValue = 512;

    QTcpSocket tcpSocket;
    tcpSocket.connectToHost(m_pHost, m_pPort);

    if (!tcpSocket.waitForConnected()) {
        qDebug() << this->currentThreadId() << "No connection";
        m_pStop = true;
    }
    else {
        qDebug() << this->currentThreadId() << "Connected";
    }

    qDebug() << this->currentThreadId() << "run" << (m_pOperation == IStorage::opRead ? "[reader]" : "[writer]");

    while (!m_pStop) {
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);

        out << int(m_pOperation);
        int index = qrand() % MaxItemsCount;
        switch (m_pOperation) {
        case IStorage::opRead: {
            out << index;            
        }
            break;
        case IStorage::opWrite: {
            out << index << qrand() % MaxItemValue;
        }
            break;
        }
        tcpSocket.write(block);
        tcpSocket.waitForBytesWritten();

        if (tcpSocket.bytesAvailable() >= sizeof(int)) {
            QDataStream in(&tcpSocket);
            int result;
            in >> result;
        }
    }
    qDebug() << this->currentThreadId() << "stop";
}
