#include "general_utility.h"
#include <QApplication>
#include <grpc_channel.h>
#include <homewindow.h>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

#ifdef QT_DEBUG
  channel = setupGrpcChannel("{redacted}", 50051);
  // channel = setupGrpcChannel("0.0.0.0", 50052);
#else
  channel = setupGrpcChannel("{redacted}", 50051);
#endif

  HomeWindow w;
  w.show();
  return QApplication::exec();
}
