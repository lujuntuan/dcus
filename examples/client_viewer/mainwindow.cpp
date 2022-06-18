#include "mainwindow.h"
#include <QAbstractButton>
#include <QClipboard>
#include <QItemDelegate>
#include <QLabel>
#include <QLine>
#include <QMessageBox>
#include <QPainter>
#include <QProgressBar>
#include <QPushButton>
#include <QTime>
#include <QTimer>

#define DOMAIN_PROGRESS_INDEX 3
#define VIEW_ITEM_HEIGHT 30
#define VIEW_PROGRESS_BAR_MAX_WIDTH 150
#define VIEW_PROGRESS_BAR_MAX_HEIGHT 20

class ViewerItemDelegate : public QItemDelegate {
public:
    using QItemDelegate::QItemDelegate;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        QSize size = QItemDelegate::sizeHint(option, index);
        size.setHeight(VIEW_ITEM_HEIGHT);
        return size;
    }
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        editor->setGeometry(option.rect.x(), option.rect.y() + ((VIEW_ITEM_HEIGHT / 2) - editor->height() / 2), option.rect.size().width(), option.rect.size().height());
    }
};

class ViewerProgressBar : public QProgressBar {
public:
    explicit ViewerProgressBar(QWidget* parent = nullptr)
        : QProgressBar(parent)
    {
        setMaximumSize(QSize(VIEW_PROGRESS_BAR_MAX_WIDTH, VIEW_PROGRESS_BAR_MAX_HEIGHT));
        setRange(0, 10000);
    }
    ~ViewerProgressBar()
    {
    }
    virtual QString text() const override
    {
        if (value() <= 0 || maximum() <= 0) {
            return QStringLiteral("0 %");
        }
        return QString::number(value() * 100.0f / maximum(), 'f', 2) + QStringLiteral(" %");
    }
    virtual void paintEvent(QPaintEvent* e) override
    {
        if (this->isEnabled()) {
            QProgressBar::paintEvent(e);
        }
    }
};

class ViewerFlag : public QWidget {
public:
    explicit ViewerFlag(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        this->setFixedSize(40, 20);
    }
    virtual void paintEvent(QPaintEvent* e) override
    {
        if (this->isEnabled()) {
            QPainter painter(this);
            painter.setRenderHint(QPainter::Antialiasing, true);
            QColor color(QStringLiteral("#078117"));
            painter.setPen(QPen(color, 0));
            painter.setBrush(color);
            painter.drawEllipse(QPoint(width() / 2, height() / 2 - 1), 8, 8);
            QWidget::paintEvent(e);
        }
    }
};

class MessageLabel : public QLabel {
public:
    using QLabel::QLabel;
    virtual void paintEvent(QPaintEvent* e) override
    {
        if (this->isEnabled()) {
            QLabel::paintEvent(e);
        }
    }
};

#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle(qApp->applicationName());
    this->resize(1280, 720);
    ViewerItemDelegate* itemDelegate = new ViewerItemDelegate(this);
    QStringList listHeaders = { QStringLiteral("Name"), QStringLiteral("State"), QStringLiteral("Version"), QStringLiteral("Progress"), QStringLiteral("Message") };
    ui->treeWidget_list->setItemDelegate(itemDelegate);
    ui->treeWidget_list->setRootIsDecorated(false);
    ui->treeWidget_list->setHeaderLabels(listHeaders);
    ui->treeWidget_list->setColumnWidth(0, 100);
    ui->treeWidget_list->setColumnWidth(1, 100);
    ui->treeWidget_list->setColumnWidth(2, 100);
    ui->treeWidget_list->setColumnWidth(3, 150);
    ui->treeWidget_list->setColumnWidth(4, 200);
    QStringList valueHeaders = { QStringLiteral("Property"), QStringLiteral("Value") };
    ui->treeWidget_value->setItemDelegate(itemDelegate);
    ui->treeWidget_value->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeWidget_value->setHeaderLabels(valueHeaders);
    ui->treeWidget_value->setColumnWidth(0, 170);
    ui->treeWidget_value->setColumnWidth(1, 300);
    connect(ui->treeWidget_list, &QTreeWidget::currentItemChanged, std::bind(&MainWindow::updateProperty, this, std::placeholders::_1));
    connect(ui->treeWidget_value, &QTreeWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
        Q_UNUSED(pos);
        if (!ui->treeWidget_value->currentItem()) {
            return;
        }
        if (ui->treeWidget_value->currentItem()->text(1).isEmpty()) {
            return;
        }
        QMenu menu;
        menu.addAction(QStringLiteral("Copy value"), this, [this]() {
            qApp->clipboard()->setText(ui->treeWidget_value->currentItem()->text(1));
        });
        menu.exec(QCursor::pos());
    });
    m_messageLabel = new MessageLabel(this);
    m_acceptBtn = new QPushButton("Yes", this);
    m_rejectBtn = new QPushButton("No", this);
    m_statusFlag = new ViewerFlag(this);
    m_messageTimer = new QTimer(this);
    m_flagTimer = new QTimer(this);
    ui->statusBar->addWidget(new QLabel("  ", this), 0);
    ui->statusBar->addWidget(m_messageLabel, 0);
    ui->statusBar->addWidget(m_acceptBtn, 0);
    ui->statusBar->addWidget(m_rejectBtn, 0);
    ui->statusBar->addPermanentWidget(m_statusFlag, 0);
    ui->statusBar->setSizeGripEnabled(false);
    ui->statusBar->setStyleSheet(QString("QStatusBar::item{border: 0px}"));
    m_messageTimer->setInterval(300);
    m_flagTimer->setInterval(100);
    connect(m_messageTimer, &QTimer::timeout, this, [this]() {
        if (m_messageLabel->isEnabled()) {
            m_messageLabel->setEnabled(false);
        } else {
            m_messageLabel->setEnabled(true);
        }
    });
    connect(m_flagTimer, &QTimer::timeout, this, [this]() {
        m_statusFlag->setEnabled(false);
        m_flagTimer->stop();
    });
    connect(m_acceptBtn, &QPushButton::clicked, this, []() {
        dcus_client_engine->postDetailAnswer(ANS_ACCEPT);
    });
    connect(m_rejectBtn, &QPushButton::clicked, this, []() {
        dcus_client_engine->postDetailAnswer(ANS_REFUSE);
    });
    QFont font = m_messageLabel->font();
    font.setBold(true);
    m_messageLabel->setFont(font);
    m_acceptBtn->setDefault(true);
    m_acceptBtn->hide();
    m_rejectBtn->hide();
    m_statusFlag->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::processDetail(const QVariant& data, bool stateChanged)
{
    const DetailMessage& detailMessage = data.value<DetailMessage>();
    processDetail(detailMessage, stateChanged);
}

void MainWindow::processDetail(const DetailMessage& detailMessage, bool stateChanged)
{
    m_statusFlag->setEnabled(true);
    m_flagTimer->stop();
    m_flagTimer->start();
    //-------------find and remove
    for (int i = 0; i < ui->treeWidget_list->topLevelItemCount(); i++) {
        auto p = ui->treeWidget_list->topLevelItem(i);
        bool find = false;
        for (const auto& d : detailMessage.details()) {
            if (p->text(0) == QString::fromStdString(d.domain().name())) {
                find = true;
                break;
            }
        }
        if (!find) {
            QWidget* widget = ui->treeWidget_list->itemWidget(p, DOMAIN_PROGRESS_INDEX);
            if (widget) {
                ui->treeWidget_list->removeItemWidget(p, DOMAIN_PROGRESS_INDEX);
                delete widget;
            }
            QTreeWidgetItem* item = ui->treeWidget_list->takeTopLevelItem(i);
            m_itemToDetail.remove(item);
            delete item;
            i--;
        }
    }
    //-------------update and create
    for (const auto& d : detailMessage.details()) {
        QTreeWidgetItem* item = nullptr;
        for (int i = 0; i < ui->treeWidget_list->topLevelItemCount(); i++) {
            auto p = ui->treeWidget_list->topLevelItem(i);
            if (p->text(0) == QString::fromStdString(d.domain().name())) {
                item = p;
            }
        }
        if (!item) {
            item = new QTreeWidgetItem(ui->treeWidget_list);
        }
        item->setText(0, QString::fromStdString(d.domain().name()));
        item->setText(1, QString::fromStdString(Domain::getWrStateStr(d.domain().state())));
        item->setText(2, QString::fromStdString(d.domain().version()));
        item->setText(3, "");
        QProgressBar* progressBar = qobject_cast<QProgressBar*>(ui->treeWidget_list->itemWidget(item, DOMAIN_PROGRESS_INDEX));
        if (!progressBar) {
            progressBar = new ViewerProgressBar(ui->treeWidget_list);
        }
        progressBar->setValue((d.progress() + 0.005f) * 100);
        if (detailMessage.state() == MR_DOWNLOAD || Domain::wrStateIsBusy(d.domain().state()) || d.progress() > 0) {
            progressBar->setEnabled(true);
        } else {
            progressBar->setEnabled(false);
        }
        ui->treeWidget_list->setItemWidget(item, DOMAIN_PROGRESS_INDEX, progressBar);
        item->setText(DOMAIN_PROGRESS_INDEX + 1, QString::fromStdString(d.domain().message()));
        ui->treeWidget_list->addTopLevelItem(item);
        m_itemToDetail.insert(item, d);
    }
    if (!ui->treeWidget_list->currentItem() && ui->treeWidget_list->topLevelItemCount() > 0) {
        ui->treeWidget_list->setCurrentItem(ui->treeWidget_list->topLevelItem(0));
        ui->treeWidget_list->setFocus();
    }
    updateProperty(ui->treeWidget_list->currentItem());
    //---------------------
    ui->lineEdit_state->setText(QString::fromStdString(Domain::getMrStateStr(detailMessage.state())));
    ui->lineEdit_last->setText(QString::fromStdString(Domain::getMrStateStr(detailMessage.last())));
    ui->lineEdit_active->setText(detailMessage.active() ? QStringLiteral("true") : QStringLiteral("false"));
    ui->lineEdit_error->setText(QString::number(detailMessage.error()));
    if (Domain::mrStateIsBusy(detailMessage.state()) || Domain::mrStateIsAsk(detailMessage.state())) {
        ui->lineEdit_action->setText(QString::fromStdString(dcus_client_engine->upgrade().id()));
        ui->lineEdit_download->setText(QString::fromStdString(Upgrade::getMethodStr(dcus_client_engine->upgrade().download())));
        ui->lineEdit_deploy->setText(QString::fromStdString(Upgrade::getMethodStr(dcus_client_engine->upgrade().deploy())));
        ui->lineEdit_maintenance->setText(dcus_client_engine->upgrade().maintenance() ? QStringLiteral("true") : QStringLiteral("false"));
        ui->label_step->setEnabled(true);
        ui->label_total->setEnabled(true);
        ui->progressBar_step->setEnabled(true);
        ui->progressBar_total->setEnabled(true);
    } else {
        ui->lineEdit_action->clear();
        ui->lineEdit_download->clear();
        ui->lineEdit_deploy->clear();
        ui->lineEdit_maintenance->clear();
        ui->label_step->setEnabled(false);
        ui->label_total->setEnabled(false);
        ui->progressBar_step->setEnabled(false);
        ui->progressBar_total->setEnabled(false);
    }
    ui->progressBar_step->setValue((detailMessage.step() + 0.005f) * 100);
    ui->progressBar_total->setValue((detailMessage.progress() + 0.005f) * 100);
    //---------------------
    if (stateChanged) {
        ui->plainTextEdit->setFocus();
        ui->plainTextEdit->appendPlainText(QString::fromStdString(detailMessage.message()));
        if (Domain::mrStateIsAsk(detailMessage.state())) {
            qApp->beep();
            switch (detailMessage.state()) {
            case MR_DOWNLOAD_ASK: {
                m_messageLabel->setText("Please confirm download.");
                m_acceptBtn->setText("Yes");
                m_rejectBtn->show();
                break;
            }
            case MR_DEPLOY_ASK: {
                m_messageLabel->setText("Please confirm deploy.");
                m_acceptBtn->setText("Yes");
                m_rejectBtn->show();
                break;
            }
            case MR_CANCEL_ASK: {
                m_messageLabel->setText("Please confirm cancel.");
                m_acceptBtn->setText("Yes");
                m_rejectBtn->show();
                break;
            }
            case MR_RESUME_ASK: {
                m_messageLabel->setText("Please confirm resume.");
                m_acceptBtn->setText("Yes");
                m_rejectBtn->show();
                break;
            }
            case MR_DONE_ASK:
                if (detailMessage.last() == MR_CANCEL) {
                    m_messageLabel->setText("Cancel successed !");
                } else {
                    m_messageLabel->setText("Upgrade successed !");
                }
                m_acceptBtn->setText("Ok");
                m_rejectBtn->hide();
                break;
            case MR_ERROR_ASK:
                if (detailMessage.last() == MR_CANCEL || detailMessage.last() == MR_CANCEL_ASK) {
                    m_messageLabel->setText("Cancel failed !");
                } else {
                    m_messageLabel->setText("Upgrade failed !");
                }
                m_acceptBtn->setText("Ok");
                m_rejectBtn->hide();
                break;
            default:
                break;
            }
            m_acceptBtn->show();
            m_acceptBtn->setDefault(true);
            m_acceptBtn->setFocus();
            m_messageTimer->start();
        } else {
            m_messageLabel->clear();
            m_acceptBtn->hide();
            m_rejectBtn->hide();
            m_messageTimer->stop();
        }
    }
}

void MainWindow::updateProperty(QTreeWidgetItem* listItem)
{
    if (!listItem) {
        ui->treeWidget_value->clear();
        return;
    }
    const Detail& d = m_itemToDetail.value(listItem);
    // updateSubProperty(nullptr, QStringLiteral("name"), QString::fromStdString(d.domain().name()));
    updateSubProperty(nullptr, QStringLiteral("guid"), QString::fromStdString(d.domain().guid()));
    // updateSubProperty(nullptr, QStringLiteral("state"), QString::fromStdString(Domain::getWrStateStr(d.domain().state())));
    updateSubProperty(nullptr, QStringLiteral("last"), QString::fromStdString(Domain::getWrStateStr(d.domain().last())));
    updateSubProperty(nullptr, QStringLiteral("watcher"), d.domain().watcher() ? QStringLiteral("true") : QStringLiteral("false"));
    updateSubProperty(nullptr, QStringLiteral("error"), QString::number(d.domain().error()));
    // updateSubProperty(nullptr, QStringLiteral("version"), QString::fromStdString(d.domain().version()));
    updateSubProperty(nullptr, QStringLiteral("attribute"), QString::fromStdString(d.domain().attribute().toJson()));
    updateSubProperty(nullptr, QStringLiteral("meta"), QString::fromStdString(d.domain().meta().toJson()));
    // updateSubProperty(nullptr, QStringLiteral("progress"), d.progress(), true);
    // updateSubProperty(nullptr, QStringLiteral("message"), QString::fromStdString(d.domain().message()));
    updateSubProperty(nullptr, QStringLiteral("deploy"), QTime(0, 0, d.deploy().get() / 1000.0).toString("hh:mm:ss"));
    QTreeWidgetItem* packagesItem = updateSubProperty(nullptr, QStringLiteral("package"), QString());
    // updateSubProperty(packagesItem, QStringLiteral("domain"), QString::fromStdString(d.package().domain()));
    updateSubProperty(packagesItem, QStringLiteral("part"), QString::fromStdString(d.package().part()));
    updateSubProperty(packagesItem, QStringLiteral("version"), QString::fromStdString(d.package().version()));
    updateSubProperty(packagesItem, QStringLiteral("meta"), QString::fromStdString(d.package().meta().toJson()));
    QTreeWidgetItem* filesItem = updateSubProperty(packagesItem, QStringLiteral("files"), QString());
    for (int i = 0; i < filesItem->childCount(); i++) {
        bool find = false;
        for (const File& file : d.package().files()) {
            if (QString::fromStdString(file.name()) == filesItem->child(i)->text(0)) {
                find = true;
                break;
            }
        }
        if (!find) {
            QTreeWidgetItem* item = filesItem->takeChild(i);
            delete item;
            i--;
        }
    }
    for (const File& file : d.package().files()) {
        QTreeWidgetItem* filesItemRoot = updateSubProperty(filesItem, QString::fromStdString(file.name()), QString());
        // updateSubProperty(filesItemRoot, QStringLiteral("domain"), QString::fromStdString(file.domain()));
        updateSubProperty(filesItemRoot, QStringLiteral("name"), QString::fromStdString(file.name()));
        updateSubProperty(filesItemRoot, QStringLiteral("url"), QString::fromStdString(file.url()));
        if (!file.md5().empty()) {
            updateSubProperty(filesItemRoot, QStringLiteral("md5"), QString::fromStdString(file.md5()));
        }
        if (!file.sha1().empty()) {
            updateSubProperty(filesItemRoot, QStringLiteral("sha1"), QString::fromStdString(file.sha1()));
        }
        if (!file.sha256().empty()) {
            updateSubProperty(filesItemRoot, QStringLiteral("sha256"), QString::fromStdString(file.sha256()));
        }
        updateSubProperty(filesItemRoot, QStringLiteral("size"), QString::number(file.size()) + QStringLiteral(" (") + QString::fromStdString(File::getSizeStr(file.size() / 1024)) + QStringLiteral(")"));
        // updateSubProperty(filesItemRoot, QStringLiteral("web_url"), QString::fromStdString(file.web_url()));
    }
    QTreeWidgetItem* transfersItem = updateSubProperty(nullptr, QStringLiteral("transfers"), QString());
    for (int i = 0; i < transfersItem->childCount(); i++) {
        bool find = false;
        for (const Transfer& transfer : d.transfers()) {
            if (QString::fromStdString(transfer.name()) == transfersItem->child(i)->text(0)) {
                find = true;
                break;
            }
        }
        if (!find) {
            QTreeWidgetItem* item = transfersItem->takeChild(i);
            auto* widget = ui->treeWidget_value->itemWidget(item, 1);
            if (widget) {
                delete widget;
            }
            delete item;
            i--;
        }
    }
    for (const Transfer& transfer : d.transfers()) {
        QTreeWidgetItem* transfersItemRoot = updateSubProperty(transfersItem, QString::fromStdString(transfer.name()), transfer.progress() + 0.005f, true);
        // updateSubProperty(transfersItemRoot, QStringLiteral("domain"), QString::fromStdString(transfer.domain()));
        updateSubProperty(transfersItemRoot, QStringLiteral("name"), QString::fromStdString(transfer.name()));
        // updateSubProperty(transfersItemRoot, QStringLiteral("progress"), transfer.progress(), true);
        updateSubProperty(transfersItemRoot, QStringLiteral("current"), QString::fromStdString(File::getSizeStr(transfer.current())));
        updateSubProperty(transfersItemRoot, QStringLiteral("total"), QString::fromStdString(File::getSizeStr(transfer.total())));
        updateSubProperty(transfersItemRoot, QStringLiteral("speed"), QString::fromStdString(File::getSizeStr(transfer.speed())) + "/S");
        updateSubProperty(transfersItemRoot, QStringLiteral("pass"), QTime(0, 0, 0).addSecs(transfer.pass()).toString("hh:mm:ss"));
        updateSubProperty(transfersItemRoot, QStringLiteral("left"), QTime(0, 0, 0).addSecs(transfer.left()).toString("hh:mm:ss"));
    }
}

QTreeWidgetItem* MainWindow::updateSubProperty(QTreeWidgetItem* item, const QString& property, const QVariant& value, bool useProgress)
{
    QTreeWidgetItem* target = nullptr;
    QProgressBar* progressBar = nullptr;
    if (item == nullptr) {
        for (int i = 0; i < ui->treeWidget_value->topLevelItemCount(); i++) {
            auto p = ui->treeWidget_value->topLevelItem(i);
            if (p->text(0) == property) {
                target = p;
                break;
            }
        }
        if (!target) {
            target = new QTreeWidgetItem(ui->treeWidget_value);
            target->setExpanded(true);
        }
    } else {
        for (int i = 0; i < item->childCount(); i++) {
            auto p = item->child(i);
            if (p->text(0) == property) {
                target = p;
                break;
            }
        }
        if (!target) {
            target = new QTreeWidgetItem(item);
            target->setExpanded(true);
        }
    }
    if (target) {
        target->setText(0, property);
    }
    if (useProgress) {
        progressBar = qobject_cast<QProgressBar*>(ui->treeWidget_value->itemWidget(target, 1));
        if (!progressBar) {
            progressBar = new ViewerProgressBar(ui->treeWidget_value);
            ui->treeWidget_value->setItemWidget(target, 1, progressBar);
        }
        progressBar->setValue(value.toFloat() * 100);
    } else {
        target->setText(1, value.toString());
    }
    return target;
}

void MainWindow::on_actionQuit_Q_triggered()
{
    this->close();
}

void MainWindow::on_actionAbout_A_triggered()
{
    QMessageBox::about(this, QStringLiteral("About DCus"),
        QStringLiteral("Domain Controller Upgrade Solution\n\nVersion: %1\nCommitID: %2")
            .arg(QStringLiteral(DCUS_PROJECT_VERSION), QStringLiteral(DCUS_PROJECT_COMMITID)));
}

void MainWindow::on_actionAboutQt_triggered()
{
    QMessageBox::aboutQt(this);
}
