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

#ifndef DCUS_WORKER_ENGINE_H
#define DCUS_WORKER_ENGINE_H

#include "dcus/base/application.h"
#include "dcus/base/domain.h"
#include "dcus/base/queue.h"
#include "dcus/base/singleton.h"
#include "dcus/worker/detail_message.h"

#define worker_engine WorkerEngine::getInstance()
#define worker_config WorkerEngine::getInstance()->config()
#define DCUS_WORKER_REGISTER(...) WorkerEngine::registerDomain(__VA_ARGS__)
#define DCUS_WORKER_CREATE(argc, argv) WorkerEngine::create(argc, argv)
#define DCUS_WORKER_DESTROY() WorkerEngine::destroy()

DCUS_NAMESPACE_BEGIN

using FilePaths = std::vector<std::string>;

class DCUS_EXPORT WorkerEngine : public Queue, public Application, public Singleton<WorkerEngine> {
public:
    using DeployFunction = std::function<void(const std::string& dir, const FilePaths& filePaths)>;
    using DetailFunction = std::function<void(const DetailMessage& detailMessage, bool stateChanged)>;
    static void registerDomain(const std::string& name, const std::string& guid = "");
    static WorkerEngine* create(int argc, char** argv);
    static void destroy();
    explicit WorkerEngine(int argc, char** argv);
    ~WorkerEngine();
    const std::string& name() const;
    const std::string& guid() const;
    const Upgrade& upgrade() const;
    Control control() const;
    bool hasCancelAction() const;
    bool hasStopAction() const;

    bool cancelEnable() const;
    const std::string& version() const;
    const Data& attribute() const;
    const Data& meta() const;
    const Data& packageMeta() const;

    void setCancelEnable(bool cancelEnable);
    void setVersion(const std::string& version);
    void setAttribute(const Data& attribute);
    void setMeta(const Data& meta);

    void subscibeDeploy(const DeployFunction& function);
    void postDeployDone(bool success, int errorCode = 0);
    void postCancelDone(bool success, int errorCode = 0);
    void postDeployProgress(float progress, const std::string& message);

    void subscibeDetail(const DetailFunction& function);
    void postDetailAnswer(Answer answer);

private:
    virtual void begin() override;
    virtual void end() override;
    virtual void eventChanged(Event* event) override;

    virtual void onStart() {};
    virtual void onStop() {};
    virtual bool onSendDomainMessage(const Domain& domain, bool discovery) = 0;

protected:
    bool needDiscovery() const;
    bool hasSubscibeDeploy() const;
    bool hasSubscibeDetail() const;
    void sendHeartbeat();
    void processControlMessage(Control control, Upgrade&& upgrade, Depends&& depends);
    void processDetailMessage(DetailMessage&& detailMessage);
    bool checkControlMessageId(uint32_t id) const;
    bool checkDetailMessageId(uint32_t id) const;

private:
    void setDomainState(WorkerState state);
    void sendDomainMessage();
    void stopThread(bool force = false);
    void download(const std::string& id, const Files& files);
    void verify(const std::string& id, const Files& files);
    void patch(const FilePaths& patchPaths);
    void deploy(const std::string& id, const Files& files);

private:
    struct WorkerHelper* m_workerHelper = nullptr;
};

DCUS_NAMESPACE_END

#endif // DCUS_WORKER_ENGINE_H
