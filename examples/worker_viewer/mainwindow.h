#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <dcus/WorkerEngine>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
class QFrame;
class QLabel;
class QTimer;
class QPushButton;
QT_END_NAMESPACE

using namespace DCus;

Q_DECLARE_METATYPE(Detail)
Q_DECLARE_METATYPE(DetailMessage)

class QTreeWidgetItem;
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

public slots:
    void processDetail(const QVariant& data, bool stateChanged);
    void processDetail(const DetailMessage& detail, bool stateChanged);
    void updateProperty(QTreeWidgetItem* listItem);

private slots:
    void on_actionQuit_Q_triggered();
    void on_actionAbout_A_triggered();
    void on_actionAboutQt_triggered();

private:
    QTreeWidgetItem* updateSubProperty(QTreeWidgetItem* item, const QString& property, const QVariant& value, bool useProgress = false);

private:
    Ui::MainWindow* ui = nullptr;
    QLabel* m_messageLabel = nullptr;
    QPushButton* m_acceptBtn = nullptr;
    QPushButton* m_rejectBtn = nullptr;
    QFrame* m_messageLine = nullptr;
    QWidget* m_statusFlag = nullptr;
    QTimer* m_flagTimer = nullptr;
    QTimer* m_messageTimer = nullptr;
};
#endif // MAINWINDOW_H
