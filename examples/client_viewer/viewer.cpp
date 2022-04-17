/*********************************************************************************
  *Copyright(C): Juntuan.Lu 2021
  *Author:  Juntuan.Lu
  *Version: 1.0
  *Date:  2021/04/22
  *Phone: 15397182986
  *Description:
  *Others:
  *Function List:
  *History:
**********************************************************************************/

#include "mainwindow.h"
#include <QApplication>
#include <QMetaMethod>

int main(int argc, char* argv[])
{
    DCUS_CLIENT_REGISTER("dcus_viewer");
    DCUS_CLIENT_CREATE(argc, argv);
    QApplication a(argc, argv);
    MainWindow window;
    client_engine->subscibeDetail([&window](const DetailMessage& detail, bool stateChanged) {
        QMetaObject::invokeMethod(&window, "processDetail", Qt::QueuedConnection, Q_ARG(QVariant, QVariant::fromValue<DetailMessage>(detail)), Q_ARG(bool, stateChanged));
    });
    client_engine->execInthread(ClientEngine::CHECK_SINGLETON);
    window.show();
    DetailMessage detailMessage;
    detailMessage.state() = MR_OFFLINE;
    window.processDetail(detailMessage, true);
    int reval = a.exec();
    client_engine->exit(reval);
    DCUS_CLIENT_DESTROY();
    return reval;
}
