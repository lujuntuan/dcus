/*********************************************************************************
 *Copyright(C): Juntuan.Lu 2022
 *Author:  Juntuan.Lu
 *Version: 1.0
 *Date:  2022/04/01
 *Phone: 15397182986
 *Description:
 *Others:
 *Function List:
 *History:
 **********************************************************************************/

#ifndef DCUS_APPLICATION_H
#define DCUS_APPLICATION_H

#include "dcus/base/data.h"
#include "dcus/base/define.h"
#include "dcus/base/log.h"
#include <vector>

DCUS_NAMESPACE_BEGIN

class DCUS_EXPORT Application {
    CLASS_DISSABLE_COPY_AND_ASSIGN(Application)
public:
    enum ApplicationFlag {
        CHECK_NONE = (1 << 1),
        CHECK_SINGLETON = (1 << 2),
        CHECK_TERMINATE = (1 << 3),
    };
    explicit Application(int argc, char** argv, const std::string& typeName = "");
    virtual ~Application();
    int argc() const;
    char** argv() const;
    const std::string& exePath() const;
    const std::string& exeDir() const;
    const std::string& exeName() const;
    const std::string& typeName() const;
    const Data& config() const;
    bool getOptions(Value& value, const Value::StringList& optNames, const std::string& configName = "") const;
    virtual void execInthread(int flag = CHECK_SINGLETON | CHECK_TERMINATE);
    virtual int exec(int flag = CHECK_SINGLETON | CHECK_TERMINATE) = 0;
    virtual void exit(int exitCode = 0) = 0;

protected:
    void loadFlagOnExec(int flag);
    void loadUseage(std::vector<Value::StringList> useage);
    Data readConfig(const std::string& fileName);

private:
    struct ApplicationHelper* m_applicationHelper = nullptr;
};

DCUS_NAMESPACE_END

#endif // DCUS_APPLICATION_H
