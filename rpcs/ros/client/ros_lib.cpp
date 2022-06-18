/*********************************************************************************
 *Copyright(C): Juntuan.Lu 2021
 *Author:  Juntuan.Lu
 *Version: 1.0
 *Date:  2022/04/01
 *Phone: 15397182986
 *Description:
 *Others:
 *Function List:
 *History:
 **********************************************************************************/

#include <dcus/ClientEngine>
#include <dcus_interfaces_ros/msg/control_message.hpp>
#include <dcus_interfaces_ros/msg/detail_message.hpp>
#include <dcus_interfaces_ros/msg/domain_message.hpp>
#include <rclcpp/rclcpp.hpp>

using namespace DCus;
using namespace dcus_interfaces_ros;

static Package transformPackage(const msg::Package& msg_package)
{
    Package package;
    package.domain() = msg_package.domain;
    package.part() = msg_package.part;
    package.version() = msg_package.version;
    package.meta() = Variant::readJson(msg_package.meta);
    for (const auto& msg_file : msg_package.files) {
        File file;
        file.domain() = msg_file.domain;
        file.name() = msg_file.name;
        file.url() = msg_file.url;
        file.size() = msg_file.size;
        file.md5() = msg_file.md5;
        file.sha1() = msg_file.sha1;
        file.sha256() = msg_file.sha256;
        package.files().push_back(std::move(file));
    }
    return package;
}

class RosClientEngine : public ClientEngine {
public:
    RosClientEngine(int argc, char** argv)
        : ClientEngine(argc, argv)
    {
        setInstance(this);
    }
    ~RosClientEngine()
    {
        setInstance(nullptr);
    }
    virtual void execInthread(int flag = CHECK_SINGLETON | CHECK_TERMINATE) override
    {
        this->loadFlagOnExec(flag);
        runInThread();
    }
    virtual int exec(int flag = CHECK_SINGLETON | CHECK_TERMINATE) override
    {
        this->loadFlagOnExec(flag);
        int reval = runInBlock();
        return reval;
    }
    virtual void exit(int exitCode = 0) override
    {
        this->quit(exitCode);
    }

protected:
    void respondControlMessage(msg::ControlMessage::UniquePtr msg_ctl)
    {
        if (checkControlMessageId(msg_ctl->id)) {
            return;
        }
        //
        Control control = (Control)msg_ctl->control;
        Upgrade upgrade;
        upgrade.id() = msg_ctl->upgrade.id;
        upgrade.download() = (Upgrade::Method)msg_ctl->upgrade.download;
        upgrade.deploy() = (Upgrade::Method)msg_ctl->upgrade.deploy;
        upgrade.maintenance() = msg_ctl->upgrade.maintenance;
        for (const auto& msg_package : msg_ctl->upgrade.packages) {
            const Package& package = transformPackage(msg_package);
            upgrade.packages().push_back(std::move(package));
        }
        Depends depends = msg_ctl->depends;
        //
        processControlMessage(control, std::move(upgrade), std::move(depends));
    }
    void respondDetailMessage(msg::DetailMessage::UniquePtr msg_dtl)
    {
        if (checkDetailMessageId(msg_dtl->id)) {
            return;
        }
        //
        DetailMessage detailMessage;
        detailMessage.state() = (ServerState)msg_dtl->state;
        detailMessage.last() = (ServerState)msg_dtl->last;
        detailMessage.active() = msg_dtl->active;
        detailMessage.error() = msg_dtl->error;
        detailMessage.step() = msg_dtl->step;
        detailMessage.progress() = msg_dtl->progress;
        detailMessage.message() = msg_dtl->message;
        for (const auto& msg_detail : msg_dtl->details) {
            Domain domain(msg_detail.domain.name, msg_detail.domain.guid);
            domain.state() = (ClientState)msg_detail.domain.state;
            domain.last() = (ClientState)msg_detail.domain.last;
            domain.watcher() = msg_detail.domain.watcher;
            domain.error() = msg_detail.domain.error;
            domain.version() = msg_detail.domain.version;
            domain.attribute() = Variant::readJson(msg_detail.domain.attribute);
            domain.meta() = Variant::readJson(msg_detail.domain.meta);
            domain.progress() = msg_detail.domain.progress;
            domain.message() = msg_detail.domain.message;
            domain.answer() = (Answer)msg_detail.domain.answer;
            Detail detail(std::move(domain));
            const Package& package = transformPackage(msg_detail.package);
            detail.package() = package;
            for (const auto& msg_transfer : msg_detail.transfers) {
                Transfer transfer;
                transfer.domain() = msg_transfer.domain;
                transfer.name() = msg_transfer.name;
                transfer.progress() = msg_transfer.progress;
                transfer.speed() = msg_transfer.speed;
                transfer.total() = msg_transfer.total;
                transfer.current() = msg_transfer.current;
                transfer.pass() = msg_transfer.pass;
                transfer.left() = msg_transfer.left;
                detail.transfers().push_back(std::move(transfer));
            }
            detail.progress() = msg_detail.progress;
            if (msg_detail.deploy > 0) {
                detail.deploy().start(Elapsed::current() - msg_detail.deploy);
            }
            detailMessage.details().push_back(std::move(detail));
        }
        processDetailMessage(std::move(detailMessage));
    }

private:
    virtual void onStart() override
    {
        Semaphore sema;
        m_rosThread.start([&]() {
            rclcpp::init(this->argc(), this->argv());
            m_rosNode = rclcpp::Node::make_shared(name());
            auto qosConfig = rclcpp::QoS(rclcpp::KeepLast(10)).reliable();
            m_rosControl = m_rosNode->create_subscription<msg::ControlMessage>("/dcus/control", qosConfig,
                std::bind(&RosClientEngine::respondControlMessage, this, std::placeholders::_1));
            if (hasSubscibeDetail()) {
                m_rosDetail = m_rosNode->create_subscription<msg::DetailMessage>("/dcus/detail", qosConfig,
                    std::bind(&RosClientEngine::respondDetailMessage, this, std::placeholders::_1));
            }
            m_rosDomain = m_rosNode->create_publisher<msg::DomainMessage>("/dcus/domain", qosConfig);
            sema.release();
            rclcpp::spin(m_rosNode);
            rclcpp::shutdown();
        });
        sema.acquire();
    }
    virtual void onStop() override
    {
        rclcpp::shutdown();
        m_rosThread.stop();
    }
    virtual bool onSendDomainMessage(const Domain& domain, bool discovery) override
    {
        if (!m_rosDomain) {
            return false;
        }
        if (m_rosDomain->get_subscription_count() <= 0) {
            return false;
        }
        msg::DomainMessage msg_domain;
        msg_domain.domain.name = domain.name();
        msg_domain.domain.guid = domain.guid();
        msg_domain.domain.state = domain.state();
        msg_domain.domain.last = domain.last();
        msg_domain.domain.watcher = domain.watcher();
        msg_domain.domain.error = domain.error();
        msg_domain.domain.version = domain.version();
        msg_domain.domain.attribute = domain.attribute().toJson();
        msg_domain.domain.meta = domain.meta().toJson();
        msg_domain.domain.progress = domain.progress();
        msg_domain.domain.message = domain.message();
        msg_domain.domain.answer = domain.answer();
        msg_domain.discovery = discovery;
        m_rosDomain->publish(msg_domain);
        return true;
    }

private:
    Thread m_rosThread;
    rclcpp::Node::SharedPtr m_rosNode;
    rclcpp::Subscription<msg::ControlMessage>::SharedPtr m_rosControl;
    rclcpp::Subscription<msg::DetailMessage>::SharedPtr m_rosDetail;
    rclcpp::Publisher<msg::DomainMessage>::SharedPtr m_rosDomain;
};

ClientEngine* ClientEngine::create(int argc, char** argv)
{
    LOG_DEBUG("rpc type: ros");
    return new RosClientEngine(argc, argv);
}
