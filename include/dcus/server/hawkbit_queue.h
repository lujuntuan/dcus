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

#ifndef DCUS_HAWKBIT_QUEUE_H
#define DCUS_HAWKBIT_QUEUE_H

#include "dcus/server/web_queue.h"

DCUS_NAMESPACE_BEGIN

class DCUS_EXPORT HawkbitQueue final : public WebQueue {
public:
    HawkbitQueue();
    ~HawkbitQueue();

protected:
    virtual bool init(const WebInit& webInit) override;
    virtual bool detect() override;
    virtual bool feedback(const WebFeed& webFeed) override;
    virtual bool transformUpgrade(Upgrade& upgrade, const std::string& jsonString) override;

private:
    struct HawkbitHelper* m_hawkbitHelper = nullptr;
};

DCUS_NAMESPACE_END

#endif //DCUS_HAWKBIT_QUEUE_H
