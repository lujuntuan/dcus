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

#include <dcus/ClientEngine>
#include <dcus/domain.h>
#include <random>

using namespace DCus;

namespace DCus {
VARIANT_DECLARE_TYPE(Domain);
}

int main(int argc, char* argv[])
{
    DCUS_CLIENT_CREATE(argc, argv);
    // dcus_client_engine->setMeta({ { "max_deploy_time", 8000 } });
    dcus_client_engine->setAttribute({ { "hw_addr", "abcdefg" } });
    dcus_client_engine->setVersion("1.0.0");
    dcus_client_engine->subscibeDeploy(
        [](const std::string& dir, const FilePaths filePaths) {
            for (int i = 0; i < 100; i++) {
                if (dcus_client_engine->hasStopAction()) {
                    return;
                }
                if (dcus_client_engine->hasCancelAction()) {
                    dcus_client_engine->postCancelDone(true);
                    return;
                }
                dcus_client_engine->postDeployProgress(
                    (float)i, std::string("Deploy progress = ") + std::to_string(i));
                static std::random_device sd;
                static std::minstd_rand linearRan(sd());
                static std::uniform_int_distribution<unsigned> round(1, 600);
                Thread::sleepMilli(round(linearRan));
            }
            dcus_client_engine->postDeployDone(true);
        });
    // deploy about 30s
    int reval = dcus_client_engine->exec(ClientEngine::CHECK_TERMINATE);
    DCUS_CLIENT_DESTROY();
    return reval;
}
