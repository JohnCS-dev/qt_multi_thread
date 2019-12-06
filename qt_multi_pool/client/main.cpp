#include "client.h"

#include <QCoreApplication>

int main(int argc, char *argv[]) {
  QCoreApplication app(argc, argv);

  {
      const QString host = "172.16.15.30";
      const int readerCount = 90;
      const int writerCount = 10;
      QList<RemoteClientThread*> readers;
      QList<RemoteClientThread*> writers;
      for (int i = 0; i < writerCount; i++) {
          writers.append(new RemoteClientThread(host, PORT, IStorage::opWrite, &app));
          writers.last()->start();
      }
      for (int i = 0; i < readerCount; i++) {
          readers.append(new RemoteClientThread(host, PORT, IStorage::opRead, &app));
          readers.last()->start();
      }

      app.exec();

      foreach (auto writer, writers) {
          writer->stop();
      }
      foreach (auto reader, readers) {
          reader->stop();
      }
  }
}
