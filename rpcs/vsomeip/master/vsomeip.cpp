/*********************************************************************************
 *Copyright(C): Juntuan.Lu 2021
 *Author:  Lu Juntuan
 *Version: 1.0
 *Date:  2021/04/22
 *Phone: 15397182986
 *Description:
 *Others:
 *Function List:
 *History:
 **********************************************************************************/

#include <CommonAPI/CommonAPI.hpp>
#include <dcus/MasterEngine>
#include <string.h>
#include <v1/commonapi/DCusInterfacesStubDefault.hpp>

using namespace DCus;
using namespace v1_0::commonapi;

static DCusInterfaces::Package transformPackage(const Package& package)
{
    DCusInterfaces::Package ci_package;
    ci_package.setDomain(package.domain());
    ci_package.setPart(package.part());
    ci_package.setVersion_(package.version());
    ci_package.setMeta(package.meta().toStream());
    DCusInterfaces::Files ci_files;
    for (const File& file : package.files()) {
        DCusInterfaces::File ci_file;
        ci_file.setDomain(file.domain());
        ci_file.setName(file.name());
        ci_file.setUrl(file.url());
        ci_file.setSize(file.size());
        ci_file.setMd5(file.md5());
        ci_file.setSha1(file.sha1());
        ci_file.setSha256(file.sha256());
        ci_files.push_back(std::move(ci_file));
    }
    ci_package.setFiles(ci_files);
    return ci_package;
}

class VSomeipMasterEngine : public MasterEngine {
public:
    VSomeipMasterEngine(int argc, char** argv)
        : MasterEngine(argc, argv)
    {
        setInstance(this);
    }
    ~VSomeipMasterEngine()
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
        controlMessage.setId(messageId);
        controlMessage.setControl((DCusInterfaces::Control::Literal)control());
        DCusInterfaces::Upgrade ci_upgrade;
        ci_upgrade.setId(upgrade().id());
        ci_upgrade.setDownload((DCusInterfaces::Method::Literal)upgrade().download());
        ci_upgrade.setDeploy((DCusInterfaces::Method::Literal)upgrade().deploy());
        ci_upgrade.setMaintenance(upgrade().maintenance());
        DCusInterfaces::Packages ci_packages;
        for (const Package& package : upgrade().packages()) {
            const DCusInterfaces::Package& ci_package = transformPackage(package);
            ci_packages.push_back(std::move(ci_package));
        }
        ci_upgrade.setPackages(std::move(ci_packages));
        controlMessage.setUpgrade(std::move(ci_upgrade));
        DCusInterfaces::Depends ci_depends;
        for (const auto& str : depends()) {
            DCusInterfaces::Depend ci_depend;
            ci_depend.setData(str);
            ci_depends.push_back(std::move(ci_depend));
        }
        controlMessage.setDepends(ci_depends);
        return controlMessage;
    }
    inline DCusInterfaces::DetailMessage getDetailMessage(uint32_t messageId) const
    {
        DCusInterfaces::DetailMessage detailMessage;
        detailMessage.setId(messageId);
        detailMessage.setState_((DCusInterfaces::MasterState::Literal)state());
        detailMessage.setLast((DCusInterfaces::MasterState::Literal)lastState());
        detailMessage.setActive(isActive());
        detailMessage.setError_(errorCode());
        detailMessage.setStep(step());
        detailMessage.setProgress(progress());
        detailMessage.setMessage(message());
        DCusInterfaces::Details ci_details;
        for (const auto& d : details()) {
            DCusInterfaces::Detail ci_detail;
            DCusInterfaces::Domain ci_domain;
            ci_domain.setName(d.domain().name());
            ci_domain.setGuid(d.domain().guid());
            ci_domain.setState_((DCusInterfaces::WorkerState::Literal)d.domain().state());
            ci_domain.setLast((DCusInterfaces::WorkerState::Literal)d.domain().last());
            ci_domain.setWatcher(d.domain().watcher());
            ci_domain.setError_(d.domain().error());
            ci_domain.setVersion_(d.domain().version());
            ci_domain.setAttribute_(d.domain().attribute().toStream());
            ci_domain.setMeta(d.domain().meta().toStream());
            ci_domain.setProgress(d.domain().progress());
            ci_domain.setMessage(d.domain().message());
            ci_domain.setAnswer((DCusInterfaces::Answer::Literal)d.domain().answer());
            ci_detail.setDomain(std::move(ci_domain));
            const DCusInterfaces::Package& ci_package = transformPackage(d.package());
            ci_detail.setPackage_(std::move(ci_package));
            DCusInterfaces::Transfers ci_transfers;
            for (const Transfer& transfer : d.transfers()) {
                DCusInterfaces::Transfer ci_transfer;
                ci_transfer.setDomain(transfer.domain());
                ci_transfer.setName(transfer.name());
                ci_transfer.setProgress(transfer.progress());
                ci_transfer.setSpeed(transfer.speed());
                ci_transfer.setTotal(transfer.total());
                ci_transfer.setCurrent(transfer.current());
                ci_transfer.setPass(transfer.pass());
                ci_transfer.setLeft(transfer.left());
                ci_transfers.push_back(std::move(ci_transfer));
            }
            ci_detail.setTransfers(std::move(ci_transfers));
            ci_detail.setProgress(d.progress());
            ci_detail.setDeploy(d.deploy().get());
            ci_details.push_back(std::move(ci_detail));
        }
        detailMessage.setDetails(std::move(ci_details));
        return detailMessage;
    }
    void respondDomainMessage(const DCusInterfaces::DomainMessage& domainMessage)
    {
        Domain domain(domainMessage.getDomain().getName(), domainMessage.getDomain().getGuid());
        domain.state() = (WorkerState)domainMessage.getDomain().getState_().value_;
        domain.last() = (WorkerState)domainMessage.getDomain().getLast().value_;
        domain.watcher() = domainMessage.getDomain().getWatcher();
        domain.error() = domainMessage.getDomain().getError_();
        domain.version() = domainMessage.getDomain().getVersion_();
        domain.attribute() = Data::readStream(domainMessage.getDomain().getAttribute_());
        domain.meta() = Data::readStream(domainMessage.getDomain().getMeta());
        domain.progress() = domainMessage.getDomain().getProgress();
        domain.message() = domainMessage.getDomain().getMessage();
        domain.answer() = (Answer)domainMessage.getDomain().getAnswer().value_;
        bool discovery = domainMessage.getDiscovery();
        processDomainMessage(std::move(domain), discovery);
    }

private:
    virtual void onStart() override
    {
        class DCusInterfaceStubImpl : public DCusInterfacesStubDefault {
        public:
            using DCusInterfacesStubDefault::DCusInterfacesStubDefault;
            virtual void invokeDomainMessage(const std::shared_ptr<CommonAPI::ClientId> _client, DCusInterfaces::DomainMessage _domainMessage) override
            {
                DCUS_UNUSED(_client);
                VSomeipMasterEngine* intance = static_cast<VSomeipMasterEngine*>(master_engine);
                intance->respondDomainMessage(_domainMessage);
            }
        };
        m_commonApiStub = std::make_shared<DCusInterfaceStubImpl>();
        CommonAPI::Runtime::get()->registerService("local", "commonapi.DCusInterfaces", m_commonApiStub, "dcus-master");
    }
    virtual void onStop() override
    {
    }
    virtual void onSendControlMessage(uint32_t messageId) override
    {
        const auto& ci_ctl = getControlMessage(messageId);
        m_commonApiStub->fireDispatchControlMessageEvent(ci_ctl);
    }
    virtual void onSendDetailMessage(uint32_t messageId) override
    {
        const auto& ci_dtl = getDetailMessage(messageId);
        m_commonApiStub->fireDispatchDetailMessageEvent(ci_dtl);
    }

private:
    std::shared_ptr<DCusInterfacesStubDefault> m_commonApiStub;
};

int main(int argc, char* argv[])
{
    VSomeipMasterEngine engine(argc, argv);
    return engine.exec();
}
