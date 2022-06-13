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

#include "dcus/base/application.h"
#include "dcus/utils/dir.h"
#include "dcus/utils/host.h"
#include "dcus/utils/string.h"
#include "dcus/utils/system.h"
#include "importlib/getopt.hpp"

#define m_hpr m_applicationHelper

DCUS_NAMESPACE_BEGIN

static Application* _intance = nullptr;
static struct getopt* _cmd_map = nullptr;

struct ApplicationHelper {
    int argc = 0;
    char** argv = nullptr;
    std::string typeName;
    std::string exePath;
    std::string exeDir;
    std::string exeName;
    Data config;
};

Application::Application(int argc, char** argv, const std::string& typeName)
{
    if (!m_hpr) {
        m_hpr = new ApplicationHelper;
    }
    m_hpr->argc = argc;
    m_hpr->argv = argv;
    m_hpr->exePath = Utils::getExePath();
    m_hpr->exeDir = Utils::getExeDir(m_hpr->exePath);
    m_hpr->exeName = Utils::getExeName(m_hpr->exePath);
    m_hpr->typeName = typeName;
    Utils::setCurrentPath(m_hpr->exeDir);
    struct getopt args(argc, argv);
    if (!_cmd_map) {
        _cmd_map = new struct getopt(argc, argv);
    }
    if (!m_hpr->typeName.empty()) {
        m_hpr->config = readConfig(m_hpr->typeName + ".conf");
        if (m_hpr->config.empty()) {
            LOG_WARNING("read config data error");
        }
    }
    dcus_print_initialize(typeName);
}

Application::~Application()
{
    if (m_hpr) {
        delete m_hpr;
        m_hpr = nullptr;
    }
    dcus_print_uninitialize();
    if (_cmd_map) {
        delete _cmd_map;
    }
}

bool Application::getOptions(Value& value, const Value::StringList& optNames, const std::string& configName) const
{
    switch (value.type()) {
    case Value::VALUE_TYPE_BOOL:
        for (const std::string& name : optNames) {
            if (_cmd_map->has(name)) {
                value = getarg<bool>(_cmd_map, value.toBool(), name.c_str());
                return true;
            }
        }
        break;
    case Value::VALUE_TYPE_INT:
        for (const std::string& name : optNames) {
            if (_cmd_map->has(name)) {
                value = getarg<int>(_cmd_map, value.toInt(), name.c_str());
                return true;
            }
        }
        break;
    case Value::VALUE_TYPE_DOUBLE:
        for (const std::string& name : optNames) {
            if (_cmd_map->has(name)) {
                value = getarg<double>(_cmd_map, value.toDouble(), name.c_str());
                return true;
            }
        }
        break;
    case Value::VALUE_TYPE_STRING:
        for (const std::string& name : optNames) {
            if (_cmd_map->has(name)) {
                value = getarg(_cmd_map, value.toStringCStr(), name.c_str());
                return true;
            }
        }
        break;
    default:
        break;
    }
    if (!configName.empty()) {
        if (m_hpr->config.value(configName).valid()) {
            value = m_hpr->config.value(configName);
            return true;
        }
    }
    return false;
}

void Application::execInthread(int flag)
{
    DCUS_UNUSED(flag);
    LOG_WARNING("not support");
}

int Application::argc() const
{
    return m_hpr->argc;
}

char** Application::argv() const
{
    return m_hpr->argv;
}

const std::string& Application::exePath() const
{
    return m_hpr->exePath;
}

const std::string& Application::exeDir() const
{
    return m_hpr->exeDir;
}

const std::string& Application::exeName() const
{
    return m_hpr->exeName;
}

const std::string& Application::typeName() const
{
    return m_hpr->typeName;
}

const Data& Application::config() const
{
    return m_hpr->config;
}

Data Application::readConfig(const std::string& fileName)
{
    Data data;
    std::string expectPath = "/etc/" + fileName;
    std::string configPath = Utils::getEnvironment("DCUS_CONF_DIR") + "/" + fileName;
    if (!Utils::exists(configPath)) {
#ifdef WIN32
        configPath = Utils::getEnvironment("ProgramFiles") + "/" + PROJECT_NAME + "/" + expectPath;
        if (!Utils::exists(configPath)) {
            configPath = Utils::getEnvironment("ProgramFiles(x86)") + "/" + PROJECT_NAME + "/" + expectPath;
#else
        configPath = expectPath;
        {
#endif
            if (!Utils::exists(configPath)) {
                configPath = m_hpr->exeDir + expectPath;
                if (!Utils::exists(configPath)) {
                    configPath = m_hpr->exeDir + "/.." + expectPath;
                    if (!Utils::exists(configPath)) {
                        // LOG_WARNING("config data not exists");
                        return data;
                    }
                }
            }
        }
    }
    data = Data::read(configPath);
    if (data.empty()) {
        // LOG_WARNING("read config data error");
        return data;
    }
    return data;
}

void Application::loadFlagOnExec(int flag)
{
    if (flag & Application::CHECK_SINGLETON) {
        if (!Utils::programCheckSingleton(m_hpr->typeName)) {
            LOG_WARNING("program is already running");
            std::quick_exit(1);
        }
    }
    if (flag & Application::CHECK_TERMINATE) {
        _intance = this;
        Utils::programRegisterTerminate([](int reval) {
            _intance->exit(reval);
        });
    }
}

void Application::loadUseage(std::vector<Value::StringList> useage)
{
    bool showVersion = getarg(_cmd_map, false, "-v", "--v", "-version", "--version");
    if (showVersion) {
        LOG_DEBUG(PROJECT_NAME, " ", PROJECT_VERSION);
        std::quick_exit(0);
    }
    bool showUseage = getarg(_cmd_map, false, "-h", "--h", "-help", "--help");
    if (showUseage) {
        std::cout << "Useage:" << std::endl;
        for (const auto& value : useage) {
            for (const auto& v : value) {
                std::cout << v << "  ";
            }
            std::cout << std::endl;
        }
        std::quick_exit(0);
    }
}

DCUS_NAMESPACE_END
