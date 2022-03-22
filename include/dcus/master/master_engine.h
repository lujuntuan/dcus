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

#ifndef DCUS_MASTER_ENGINE_H
#define DCUS_MASTER_ENGINE_H

#include "dcus/base/application.h"
#include "dcus/base/data.h"
#include "dcus/base/domain.h"
#include "dcus/base/queue.h"
#include "dcus/base/singleton.h"
#include "dcus/base/upgrade.h"

#define master_engine MasterEngine::getInstance()
#define master_config MasterEngine::getInstance()->config()

DCUS_NAMESPACE_BEGIN

class DCUS_EXPORT MasterEngine : public Queue, public Application, public Singleton<MasterEngine> {
public:
    explicit MasterEngine(int argc, char** argv);
    virtual ~MasterEngine();

    const Upgrade& upgrade() const;
    const Details& details() const;
    const Depends& depends() const;
    const std::string& cancelId() const;
    const std::string& message() const;
    const Data attributes() const;
    MasterState state() const;
    MasterState lastState() const;
    Control control() const;
    int errorCode() const;
    bool isActive() const;
    float step() const;
    float progress() const;

    void startWebEngine();
    void stopWebEngine();
    void updateMessage();
    void processDomainMessage(Domain&& domain, bool discovery);

private:
    virtual void begin() override;
    virtual void end() override;
    virtual void eventChanged(Event* event) override;

    virtual void onStart() {};
    virtual void onStop() {};
    virtual void onSendControlMessage(uint32_t messageId) = 0;
    virtual void onSendDetailMessage(uint32_t messageId) = 0;

private:
    std::vector<std::string> getWebFeedDetails() const;
    std::pair<int, int> getWebFeedProgress() const;
    void configState(bool isLastError = false);
    void configAnswer(Answer answer);
    void setState(MasterState state);
    void readState();
    void resume();
    void idle();
    void pending();
    void download();
    void deploy();
    void cancel();
    void feedback(bool finished, int error = 0);
    void sendControlMessage(Control control, bool cache = false);
    void sendDetailMessage(bool cache = false);
    void processDomains();

private:
    struct MasterHelper* m_masterHelper = nullptr;
    friend class WebQueue;
};

DCUS_NAMESPACE_END

#endif // DCUS_MASTER_ENGINE_H
