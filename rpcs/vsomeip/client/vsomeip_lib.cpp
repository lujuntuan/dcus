/*********************************************************************************
 *Copyright(C): Juntuan.Lu 2021
 *Author:  Lu Juntuan
 *Version: 1.0
 *Date:  2022/04/01
 *Phone: 15397182986
 *Description:
 *Others:
 *Function List:
 *History:
 **********************************************************************************/

#include <CommonAPI/CommonAPI.hpp>
#include <dcus/ClientEngine>
#include <string.h>
#include <v1/commonapi/DCusInterfacesProxy.hpp>

using namespace DCus;
using namespace v1_0::commonapi;

static Package transformPackage(const DCusInterfaces::Package& ci_package)
{
    Package package;
    package.domain() = ci_package.getDomain();
    package.part() = ci_package.getPart();
    package.version() = ci_package.getVersion_();
    package.meta() = Data::readStream(ci_package.getMeta());
    for (const auto& ci_file : ci_package.getFiles()) {
        File file;
        file.domain() = ci_file.getDomain();
        file.name() = ci_file.getName();
        file.url() = ci_file.getUrl();
        file.size() = ci_file.getSize();
        file.md5() = ci_file.getMd5();
        file.sha1() = ci_file.getSha1();
        file.sha256() = ci_file.getSha256();
        package.files().push_back(std::move(file));
    }
    return package;
}

class VSomeipClientEngine : public ClientEngine {
public:
    VSomeipClientEngine(int argc, char** argv)
        : ClientEngine(argc, argv)
    {
        setInstance(this);
    }
    ~VSomeipClientEngine()
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
    void subscribeVSomeip()
    {
    }
    void respondControlMessage(const DCusInterfaces::ControlMessage& ci_ctl)
    {
        if (checkControlMessageId(ci_ctl.getId())) {
            return;
        }
        //
        Control control = (Control)ci_ctl.getControl().value_;
        Upgrade upgrade;
        upgrade.id() = ci_ctl.getUpgrade().getId();
        upgrade.download() = (Upgrade::Method)ci_ctl.getUpgrade().getDownload().value_;
        upgrade.deploy() = (Upgrade::Method)ci_ctl.getUpgrade().getDeploy().value_;
        upgrade.maintenance() = ci_ctl.getUpgrade().getMaintenance();
        for (const auto& ci_package : ci_ctl.getUpgrade().getPackages()) {
            const Package& package = transformPackage(ci_package);
            upgrade.packages().push_back(std::move(package));
        }
        Depends depends;
        for (const auto& ci_depend : ci_ctl.getDepends()) {
            depends.push_back(std::move(ci_depend.getData()));
        }
        //
        processControlMessage(control, std::move(upgrade), std::move(depends));
    }
    void respondDetailMessage(const DCusInterfaces::DetailMessage& ci_dtl)
    {
        if (checkDetailMessageId(ci_dtl.getId())) {
            return;
        }
        //
        DetailMessage detailMessage;
        detailMessage.state() = (ServerState)ci_dtl.getState_().value_;
        detailMessage.last() = (ServerState)ci_dtl.getLast().value_;
        detailMessage.active() = ci_dtl.getActive();
        detailMessage.error() = ci_dtl.getError_();
        detailMessage.step() = ci_dtl.getStep();
        detailMessage.progress() = ci_dtl.getProgress();
        detailMessage.message() = ci_dtl.getMessage();
        for (const auto& ci_detail : ci_dtl.getDetails()) {
            Domain domain(ci_detail.getDomain().getName(), ci_detail.getDomain().getGuid());
            domain.state() = (ClientState)ci_detail.getDomain().getState_().value_;
            domain.last() = (ClientState)ci_detail.getDomain().getLast().value_;
            domain.watcher() = ci_detail.getDomain().getWatcher();
            domain.error() = ci_detail.getDomain().getError_();
            domain.version() = ci_detail.getDomain().getVersion_();
            domain.attribute() = Data::readStream(ci_detail.getDomain().getAttribute_());
            domain.meta() = Data::readStream(ci_detail.getDomain().getMeta());
            domain.progress() = ci_detail.getDomain().getProgress();
            domain.message() = ci_detail.getDomain().getMessage();
            domain.answer() = (Answer)ci_detail.getDomain().getAnswer().value_;
            Detail detail(std::move(domain));
            const Package& package = transformPackage(ci_detail.getPackage_());
            detail.package() = package;
            for (const auto& ci_transfer : ci_detail.getTransfers()) {
                Transfer transfer;
                transfer.domain() = ci_transfer.getDomain();
                transfer.name() = ci_transfer.getName();
                transfer.progress() = ci_transfer.getProgress();
                transfer.speed() = ci_transfer.getSpeed();
                transfer.total() = ci_transfer.getTotal();
                transfer.current() = ci_transfer.getCurrent();
                transfer.pass() = ci_transfer.getPass();
                transfer.left() = ci_transfer.getLeft();
                detail.transfers().push_back(std::move(transfer));
            }
            detail.progress() = ci_detail.getProgress();
            if (ci_detail.getDeploy() > 0) {
                detail.deploy().start(Elapsed::current() - ci_detail.getDeploy());
            }
            detailMessage.details().push_back(std::move(detail));
        }
        processDetailMessage(std::move(detailMessage));
    }

private:
    virtual void onStart() override
    {
        m_commonApiProxy = CommonAPI::Runtime::get()->buildProxy<DCusInterfacesProxy>("local", "commonapi.DCusInterfaces", "dcus-client");
        m_commonApiProxy->getDispatchControlMessageEvent().subscribe([this](const DCusInterfaces::ControlMessage& ci_ctl) {
            respondControlMessage(ci_ctl);
        });
        if (hasSubscibeDetail()) {
            m_commonApiProxy->getDispatchDetailMessageEvent().subscribe([this](const DCusInterfaces::DetailMessage& ci_dtl) {
                respondDetailMessage(ci_dtl);
            });
        }
    }
    virtual void onStop() override
    {
    }
    virtual bool onSendDomainMessage(const Domain& domain, bool discovery) override
    {
        if (!m_commonApiProxy) {
            return false;
        }
        if (!m_commonApiProxy->isAvailable()) {
            return false;
        }
        DCusInterfaces::DomainMessage domainMessage;
        DCusInterfaces::Domain ci_domain;
        ci_domain.setName(domain.name());
        ci_domain.setGuid(domain.guid());
        ci_domain.setState_((DCusInterfaces::ClientState::Literal)domain.state());
        ci_domain.setLast((DCusInterfaces::ClientState::Literal)domain.last());
        ci_domain.setWatcher(domain.watcher());
        ci_domain.setError_(domain.error());
        ci_domain.setVersion_(domain.version());
        ci_domain.setAttribute_(domain.attribute().toStream());
        ci_domain.setMeta(domain.meta().toStream());
        ci_domain.setProgress(domain.progress());
        ci_domain.setMessage(domain.message());
        ci_domain.setAnswer((DCusInterfaces::Answer::Literal)domain.answer());
        domainMessage.setDomain(std::move(ci_domain));
        domainMessage.setDiscovery(discovery);
        CommonAPI::CallStatus callStatus(CommonAPI::CallStatus::SUCCESS);
        m_commonApiProxy->invokeDomainMessage(domainMessage, callStatus);
        return true;
    }

private:
    std::shared_ptr<DCusInterfacesProxy<>> m_commonApiProxy;
};

ClientEngine* ClientEngine::create(int argc, char** argv)
{
    LOG_DEBUG("rpc type: vsomeip");
    return new VSomeipClientEngine(argc, argv);
}
