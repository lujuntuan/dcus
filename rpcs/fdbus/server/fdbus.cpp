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

#include <DCusInterfaces.pb.h>
#include <common_base/CFdbProtoMsgBuilder.h>
#include <common_base/fdbus.h>
#include <dcus/ServerEngine>

using namespace DCus;

static DCusInterfaces::Package transformPackage(const Package& package)
{
    DCusInterfaces::Package pb_package;
    pb_package.set_domain(package.domain());
    pb_package.set_part(package.part());
    pb_package.set_version(package.version());
    pb_package.set_meta(package.meta().toStream());
    for (const File& file : package.files()) {
        auto* pb_fileList = pb_package.add_files();
        pb_fileList->set_domain(file.domain());
        pb_fileList->set_name(file.name());
        pb_fileList->set_url(file.url());
        pb_fileList->set_size(file.size());
        pb_fileList->set_md5(file.md5());
        pb_fileList->set_sha1(file.sha1());
        pb_fileList->set_sha256(file.sha256());
    }
    return pb_package;
}

class FdbusServerEngine : public ServerEngine, public CBaseServer {
public:
    FdbusServerEngine(int argc, char** argv)
        : ServerEngine(argc, argv)
        , CBaseServer(DCUS_SERVER_NAME)
    {
        setInstance(this);
        FDB_CONTEXT->init();
    }
    ~FdbusServerEngine()
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
    inline DCusInterfaces::ControlMessage getControlMessage(uint32_t messageId) const
    {
        DCusInterfaces::ControlMessage controlMessage;
        controlMessage.set_id(messageId);
        controlMessage.set_control((DCusInterfaces::ControlMessage_Control)control());
        controlMessage.mutable_upgrade()->set_id(upgrade().id());
        controlMessage.mutable_upgrade()->set_download((DCusInterfaces::ControlMessage_Upgrade_Method)upgrade().download());
        controlMessage.mutable_upgrade()->set_deploy((DCusInterfaces::ControlMessage_Upgrade_Method)upgrade().deploy());
        controlMessage.mutable_upgrade()->set_maintenance(upgrade().maintenance());
        for (const Package& package : upgrade().packages()) {
            auto* pb_package = controlMessage.mutable_upgrade()->add_packages();
            *pb_package = transformPackage(package);
        }
        for (const std::string& n : depends()) {
            auto* pb_n = controlMessage.add_depends();
            *pb_n = n;
        }
        return controlMessage;
    }
    inline DCusInterfaces::DetailMessage getDetailMessage(uint32_t messageId) const
    {
        DCusInterfaces::DetailMessage detailMessage;
        detailMessage.set_id(messageId);
        detailMessage.set_state((DCusInterfaces::DetailMessage_ServerState)state());
        detailMessage.set_last((DCusInterfaces::DetailMessage_ServerState)lastState());
        detailMessage.set_active(isActive());
        detailMessage.set_error(errorCode());
        detailMessage.set_step(step());
        detailMessage.set_progress(progress());
        detailMessage.set_message(message());
        for (const auto& d : details()) {
            auto* pb_detail = detailMessage.add_details();
            pb_detail->mutable_domain()->set_name(d.domain().name());
            pb_detail->mutable_domain()->set_guid(d.domain().guid());
            pb_detail->mutable_domain()->set_state((DCusInterfaces::Domain_ClientState)d.domain().state());
            pb_detail->mutable_domain()->set_last((DCusInterfaces::Domain_ClientState)d.domain().last());
            pb_detail->mutable_domain()->set_watcher(d.domain().watcher());
            pb_detail->mutable_domain()->set_error(d.domain().error());
            pb_detail->mutable_domain()->set_version(d.domain().version());
            pb_detail->mutable_domain()->set_attribute(d.domain().attribute().toStream());
            pb_detail->mutable_domain()->set_meta(d.domain().meta().toStream());
            pb_detail->mutable_domain()->set_progress(d.domain().progress());
            pb_detail->mutable_domain()->set_message(d.domain().message());
            pb_detail->mutable_domain()->set_answer((DCusInterfaces::Domain_Answer)d.domain().answer());
            *(pb_detail->mutable_package()) = transformPackage(d.package());
            for (const Transfer& transfer : d.transfers()) {
                auto* pb_transfer = pb_detail->add_transfers();
                pb_transfer->set_domain(transfer.domain());
                pb_transfer->set_name(transfer.name());
                pb_transfer->set_progress(transfer.progress());
                pb_transfer->set_speed(transfer.speed());
                pb_transfer->set_total(transfer.total());
                pb_transfer->set_current(transfer.current());
                pb_transfer->set_pass(transfer.pass());
                pb_transfer->set_left(transfer.left());
            }
            pb_detail->set_progress(d.progress());
            pb_detail->set_deploy(d.deploy().get());
        }
        return detailMessage;
    }

private:
    virtual void onStart() override
    {
        FDB_CONTEXT->start();
        this->bind(FDB_CONNECT_ADDRESS);
    }
    virtual void onStop() override
    {
        this->unbind();
    }
    virtual void onSendControlMessage(uint32_t messageId) override
    {
        const auto& controlMessage = getControlMessage(messageId);
        CFdbProtoMsgBuilder builder(controlMessage);
        this->broadcast(DCusInterfaces::TP_CONTROL_MSG, builder);
    }
    virtual void onSendDetailMessage(uint32_t messageId) override
    {
        const auto& detailMessage = getDetailMessage(messageId);
        CFdbProtoMsgBuilder builder(detailMessage);
        this->broadcast(DCusInterfaces::TP_DETAIL_MSG, builder);
    }
    void onOnline(FdbSessionId_t sid, bool is_first) override
    {
        DCUS_UNUSED(is_first);
        m_sid = sid;
    }
    void onOffline(FdbSessionId_t sid, bool is_last) override
    {
        DCUS_UNUSED(is_last);
        m_sid = -1;
    }
    void onSubscribe(CBaseJob::Ptr& msg_ref) override
    {
        CFdbMessage* msgData = castToMessage<CBaseMessage*>(msg_ref);
        CFdbSession* session = FDB_CONTEXT->getSession(msgData->session());
        if (!session) {
            return;
        }
    }
    void onInvoke(CBaseJob::Ptr& msg_ref) override
    {
        CFdbMessage* msgData = castToMessage<CBaseMessage*>(msg_ref);
        CFdbSession* session = FDB_CONTEXT->getSession(msgData->session());
        if (!session) {
            return;
        }
        if (msgData->code() == DCusInterfaces::TP_DOMAIN_MSG) {
            DCusInterfaces::DomainMessage domainMessage;
            if (msgData->getPayloadSize() > 0) {
                CFdbProtoMsgParser parser(domainMessage);
                if (!msgData->deserialize(parser)) {
                    LOG_WARNING("deserialize msg error");
                    return;
                }
            }
            Domain domain(domainMessage.domain().name(), domainMessage.domain().guid());
            domain.state() = (ClientState)domainMessage.domain().state();
            domain.last() = (ClientState)domainMessage.domain().last();
            domain.watcher() = domainMessage.domain().watcher();
            domain.error() = domainMessage.domain().error();
            domain.version() = domainMessage.domain().version();
            domain.attribute() = Data::readStream(domainMessage.domain().attribute());
            domain.meta() = Data::readStream(domainMessage.domain().meta());
            domain.progress() = domainMessage.domain().progress();
            domain.message() = domainMessage.domain().message();
            domain.answer() = (Answer)domainMessage.domain().answer();
            bool discovery = domainMessage.discovery();
            processDomainMessage(std::move(domain), discovery);
        }
    }

private:
    FdbSessionId_t m_sid = -1;
};

int main(int argc, char* argv[])
{
    FdbusServerEngine engine(argc, argv);
    return engine.exec();
}
