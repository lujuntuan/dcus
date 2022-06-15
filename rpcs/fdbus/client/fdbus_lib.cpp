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
#include <dcus/ClientEngine>

using namespace DCus;

static Package transformPackage(const DCusInterfaces::Package& pb_package)
{
    Package package;
    package.domain() = pb_package.domain();
    package.part() = pb_package.part();
    package.version() = pb_package.version();
    package.meta() = Data::readStream(pb_package.meta());
    for (int i = 0; i < pb_package.files_size(); i++) {
        File file;
        const auto& pb_file = pb_package.files(i);
        file.domain() = pb_file.domain();
        file.name() = pb_file.name();
        file.url() = pb_file.url();
        file.size() = pb_file.size();
        file.md5() = pb_file.md5();
        file.sha1() = pb_file.sha1();
        file.sha256() = pb_file.sha256();
        package.files().push_back(std::move(file));
    }
    return package;
}

class FdbusClientEngine : public ClientEngine, public CBaseClient {
public:
    FdbusClientEngine(int argc, char** argv)
        : ClientEngine(argc, argv)
        , CBaseClient(this->ClientEngine::name().c_str())
    {
        setInstance(this);
        FDB_CONTEXT->init();
    }
    ~FdbusClientEngine()
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
    void respondControlMessage(const DCusInterfaces::ControlMessage& pb_ctl)
    {
        if (checkControlMessageId(pb_ctl.id())) {
            return;
        }
        //
        Control control = (Control)pb_ctl.control();
        Upgrade upgrade;
        upgrade.id() = pb_ctl.upgrade().id();
        upgrade.download() = (Upgrade::Method)pb_ctl.upgrade().download();
        upgrade.deploy() = (Upgrade::Method)pb_ctl.upgrade().deploy();
        upgrade.maintenance() = pb_ctl.upgrade().maintenance();
        for (int i = 0; i < pb_ctl.upgrade().packages_size(); i++) {
            const auto& pb_package = pb_ctl.upgrade().packages(i);
            const Package& package = transformPackage(pb_package);
            upgrade.packages().push_back(std::move(package));
        }
        Depends depends;
        for (int i = 0; i < pb_ctl.depends_size(); i++) {
            const auto& pb_name = pb_ctl.depends(i);
            depends.push_back(std::move(pb_name));
        }
        //
        processControlMessage(control, std::move(upgrade), std::move(depends));
    }
    void respondDetailMessage(const DCusInterfaces::DetailMessage& pb_dtl)
    {
        if (checkDetailMessageId(pb_dtl.id())) {
            return;
        }
        //
        DetailMessage detailMessage;
        detailMessage.state() = (ServerState)pb_dtl.state();
        detailMessage.last() = (ServerState)pb_dtl.last();
        detailMessage.active() = pb_dtl.active();
        detailMessage.error() = pb_dtl.error();
        detailMessage.step() = pb_dtl.step();
        detailMessage.progress() = pb_dtl.progress();
        detailMessage.message() = pb_dtl.message();
        for (int i = 0; i < pb_dtl.details_size(); i++) {
            const auto& pb_detail = pb_dtl.details(i);
            Domain domain(pb_detail.domain().name(), pb_detail.domain().guid());
            domain.state() = (ClientState)pb_detail.domain().state();
            domain.last() = (ClientState)pb_detail.domain().last();
            domain.watcher() = pb_detail.domain().watcher();
            domain.error() = pb_detail.domain().error();
            domain.version() = pb_detail.domain().version();
            domain.attribute() = Data::readStream(pb_detail.domain().attribute());
            domain.meta() = Data::readStream(pb_detail.domain().meta());
            domain.progress() = pb_detail.domain().progress();
            domain.message() = pb_detail.domain().message();
            domain.answer() = (Answer)pb_detail.domain().answer();
            Detail detail(std::move(domain));
            const Package& package = transformPackage(pb_detail.package());
            detail.package() = package;
            for (int j = 0; j < pb_detail.transfers_size(); j++) {
                const auto& pb_transfer = pb_detail.transfers(j);
                Transfer transfer;
                transfer.domain() = pb_transfer.domain();
                transfer.name() = pb_transfer.name();
                transfer.progress() = pb_transfer.progress();
                transfer.speed() = pb_transfer.speed();
                transfer.total() = pb_transfer.total();
                transfer.current() = pb_transfer.current();
                transfer.pass() = pb_transfer.pass();
                transfer.left() = pb_transfer.left();
                detail.transfers().push_back(std::move(transfer));
            }
            detail.progress() = pb_detail.progress();
            if (pb_detail.deploy() > 0) {
                detail.deploy().start(Elapsed::current() - pb_detail.deploy());
            }
            detailMessage.details().push_back(std::move(detail));
        }
        processDetailMessage(std::move(detailMessage));
    }

private:
    virtual void onStart() override
    {
        FDB_CONTEXT->start();
        this->connect(FDB_CONNECT_ADDRESS);
    }
    virtual void onStop() override
    {
        this->disconnect();
    }
    virtual bool onSendDomainMessage(const Domain& domain, bool discovery) override
    {
        if (m_sid < 0) {
            return false;
        }
        DCusInterfaces::DomainMessage domainMessage;
        domainMessage.mutable_domain()->set_name(domain.name());
        domainMessage.mutable_domain()->set_guid(domain.guid());
        domainMessage.mutable_domain()->set_state((DCusInterfaces::Domain_ClientState)domain.state());
        domainMessage.mutable_domain()->set_last((DCusInterfaces::Domain_ClientState)domain.last());
        domainMessage.mutable_domain()->set_watcher(domain.watcher());
        domainMessage.mutable_domain()->set_error(domain.error());
        domainMessage.mutable_domain()->set_version(domain.version());
        domainMessage.mutable_domain()->set_attribute(domain.attribute().toStream());
        domainMessage.mutable_domain()->set_meta(domain.meta().toStream());
        domainMessage.mutable_domain()->set_progress(domain.progress());
        domainMessage.mutable_domain()->set_message(domain.message());
        domainMessage.mutable_domain()->set_answer((DCusInterfaces::Domain_Answer)domain.answer());
        domainMessage.set_discovery(discovery);
        CFdbProtoMsgBuilder builder(domainMessage);
        invoke(DCusInterfaces::TP_DOMAIN_MSG, builder);
        return true;
    }
    virtual void onOnline(FdbSessionId_t sid, bool is_first) override
    {
        DCUS_UNUSED(is_first);
        CFdbMsgSubscribeList subList;
        addNotifyItem(subList, DCusInterfaces::TP_CONTROL_MSG);
        if (hasSubscibeDetail()) {
            addNotifyItem(subList, DCusInterfaces::TP_DETAIL_MSG);
        }
        subscribe(subList);
        m_sid = sid;
    }
    virtual void onOffline(FdbSessionId_t sid, bool is_last) override
    {
        DCUS_UNUSED(is_last);
        m_sid = -1;
    }
    virtual void onBroadcast(CBaseJob::Ptr& msg_ref) override
    {
        CFdbMessage* msgData = castToMessage<CBaseMessage*>(msg_ref);
        if (!msgData) {
            return;
        }
        CFdbSession* session = FDB_CONTEXT->getSession(msgData->session());
        if (!session) {
            return;
        }
        if (msgData->code() == DCusInterfaces::TP_CONTROL_MSG) {
            DCusInterfaces::ControlMessage pb_ctl;
            if (msgData->getPayloadSize() > 0) {
                CFdbProtoMsgParser parser(pb_ctl);
                if (!msgData->deserialize(parser)) {
                    LOG_WARNING("deserialize msg error");
                    return;
                }
            }
            respondControlMessage(pb_ctl);
        } else if (msgData->code() == DCusInterfaces::TP_DETAIL_MSG) {
            DCusInterfaces::DetailMessage pb_dtl;
            if (msgData->getPayloadSize() > 0) {
                CFdbProtoMsgParser parser(pb_dtl);
                if (!msgData->deserialize(parser)) {
                    LOG_WARNING("deserialize msg error");
                    return;
                }
            }
            respondDetailMessage(pb_dtl);
        }
    }

private:
    FdbSessionId_t m_sid = -1;
};

ClientEngine* ClientEngine::create(int argc, char** argv)
{
    LOG_DEBUG("rpc type: fdbus");
    return new FdbusClientEngine(argc, argv);
}
