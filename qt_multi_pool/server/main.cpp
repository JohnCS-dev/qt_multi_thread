#include "server.h"
#include <QCoreApplication>

int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);
  StorageServer server(PORT, &app);
  app.exec();
}
