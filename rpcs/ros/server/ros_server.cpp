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

#include <dcus/ServerEngine>
#include <dcus_interfaces_ros/msg/control_message.hpp>
#include <dcus_interfaces_ros/msg/detail_message.hpp>
#include <dcus_interfaces_ros/msg/domain_message.hpp>
#include <rclcpp/rclcpp.hpp>

using namespace DCus;
using namespace dcus_interfaces_ros;

static msg::Package transformPackage(const Package& package)
{
    msg::Package msg_package;
    msg_package.domain = package.domain;
    msg_package.part = package.part;
    msg_package.version = package.version;
    msg_package.meta = package.meta.toJson();
    for (const File& file : package.files) {
        msg::File msg_file;
        msg_file.domain = file.domain;
        msg_file.name = file.name;
        msg_file.url = file.url;
        msg_file.size = file.size;
        msg_file.md5 = file.md5;
        msg_file.sha1 = file.sha1;
        msg_file.sha256 = file.sha256;
        msg_package.files.push_back(std::move(msg_file));
    }
    return msg_package;
}

class RosServerEngine : public ServerEngine {
public:
    RosServerEngine(int argc, char** argv)
        : ServerEngine(argc, argv)
    {
        setInstance(this);
    }
    ~RosServerEngine()
    {
        setInstance(nullptr);
    }
    virtual int exec(int flag = CHECK_SINGLETON | CHECK_TERMINATE) override
    {
        this->loadFlagOnExec(flag);
        this->startWebEngine();
        int reval = runInBlock();
        return reval;
    }
    virtual void exit(int exitCode = 0) override
    {
        this->stopWebEngine();
        this->quit(exitCode);
    }

protected:
    inline msg::ControlMessage getControlMessage(uint32_t messageId) const
    {
        msg::ControlMessage msg_ctl;
        msg_ctl.id = messageId;
        msg_ctl.control = control();
        msg_ctl.upgrade.id = upgrade().id;
        msg_ctl.upgrade.download = upgrade().download;
        msg_ctl.upgrade.deploy = upgrade().deploy;
        msg_ctl.upgrade.maintenance = upgrade().maintenance;
        for (const auto& package : upgrade().packages) {
            msg_ctl.upgrade.packages.push_back(transformPackage(package));
        }
        msg_ctl.depends = depends();
        return msg_ctl;
    }

    inline msg::DetailMessage getDetailMessage(uint32_t messageId) const
    {
        msg::DetailMessage msg_dtl;
        msg_dtl.id = messageId;
        msg_dtl.state = state();
        msg_dtl.last = lastState();
        msg_dtl.active = isActive();
        msg_dtl.error = errorCode();
        msg_dtl.step = step();
        msg_dtl.progress = progress();
        msg_dtl.message = message();
        for (const auto& d : details()) {
            msg::Detail msg_detail;
            msg_detail.domain.name = d.domain.name;
            msg_detail.domain.guid = d.domain.guid;
            msg_detail.domain.state = d.domain.state;
            msg_detail.domain.last = d.domain.last;
            msg_detail.domain.watcher = d.domain.watcher;
            msg_detail.domain.error = d.domain.error;
            msg_detail.domain.version = d.domain.version;
            msg_detail.domain.attribute = d.domain.attribute.toJson();
            msg_detail.domain.meta = d.domain.meta.toJson();
            msg_detail.domain.progress = d.domain.progress;
            msg_detail.domain.message = d.domain.message;
            msg_detail.domain.answer = d.domain.answer;
            msg_detail.package = transformPackage(d.package);
            for (const Transfer& transfer : d.transfers) {
                msg::Transfer msg_transfer;
                msg_transfer.domain = transfer.domain;
                msg_transfer.name = transfer.name;
                msg_transfer.progress = transfer.progress;
                msg_transfer.speed = transfer.speed;
                msg_transfer.total = transfer.total;
                msg_transfer.current = transfer.current;
                msg_transfer.pass = transfer.pass;
                msg_transfer.left = transfer.left;
                msg_detail.transfers.push_back(std::move(msg_transfer));
            }
            msg_detail.progress = d.progress;
            msg_detail.deploy = d.deploy.get();
            msg_dtl.details.push_back(std::move(msg_detail));
        }
        return msg_dtl;
    }
    void respondDomainMessage(msg::DomainMessage::UniquePtr msg_domain)
    {
        Domain domain(msg_domain->domain.name, msg_domain->domain.guid);
        domain.state = (ClientState)msg_domain->domain.state;
        domain.last = (ClientState)msg_domain->domain.last;
        domain.watcher = msg_domain->domain.watcher;
        domain.error = msg_domain->domain.error;
        domain.version = msg_domain->domain.version;
        domain.attribute = Variant::readJson(msg_domain->domain.attribute);
        domain.meta = Variant::readJson(msg_domain->domain.meta);
        domain.progress = msg_domain->domain.progress;
        domain.message = msg_domain->domain.message;
        domain.answer = (Answer)msg_domain->domain.answer;
        bool discovery = msg_domain->discovery;
        processDomainMessage(std::move(domain), discovery);
    }

private:
    virtual void onStart() override
    {
        Semaphore sema;
        m_rosThread.start([&]() {
            rclcpp::init(this->argc(), this->argv());
            m_rosNode = rclcpp::Node::make_shared(DCUS_SERVER_NAME);
            auto qosConfig = rclcpp::QoS(rclcpp::KeepLast(10)).reliable();
            m_rosControl = m_rosNode->create_publisher<msg::ControlMessage>("/dcus/control", qosConfig);
            m_rosDetail = m_rosNode->create_publisher<msg::DetailMessage>("/dcus/detail", qosConfig);
            m_rosDomain = m_rosNode->create_subscription<msg::DomainMessage>("/dcus/domain", qosConfig,
                std::bind(&RosServerEngine::respondDomainMessage, this, std::placeholders::_1));
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
    virtual void onSendControlMessage(uint32_t messageId) override
    {
        if (!m_rosControl) {
            return;
        }
        const auto& controlMessage = getControlMessage(messageId);
        m_rosControl->publish(controlMessage);
    }
    virtual void onSendDetailMessage(uint32_t messageId) override
    {
        if (!m_rosDetail) {
            return;
        }
        const auto& detailMessage = getDetailMessage(messageId);
        m_rosDetail->publish(detailMessage);
    }

private:
    Thread m_rosThread;
    rclcpp::Node::SharedPtr m_rosNode;
    rclcpp::Publisher<msg::ControlMessage>::SharedPtr m_rosControl;
    rclcpp::Publisher<msg::DetailMessage>::SharedPtr m_rosDetail;
    rclcpp::Subscription<msg::DomainMessage>::SharedPtr m_rosDomain;
};

int main(int argc, char* argv[])
{
    RosServerEngine engine(argc, argv);
    return engine.exec();
}
