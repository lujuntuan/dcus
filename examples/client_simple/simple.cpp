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

#include <dcus/ClientEngine>
#include <random>

using namespace DCus;

int main(int argc, char* argv[])
{
    DCUS_CLIENT_CREATE(argc, argv);
    // client_engine->setMeta(Data({ { "max_deploy_time", 8000 } }));
    client_engine->setAttribute(Data({ { "hw_addr", "abcdefg" } }));
    client_engine->setVersion("1.0.0");
    client_engine->subscibeDeploy(
        [](const std::string& dir, const FilePaths filePaths) {
            for (int i = 0; i < 100; i++) {
                if (client_engine->hasStopAction()) {
                    return;
                }
                if (client_engine->hasCancelAction()) {
                    client_engine->postCancelDone(true);
                    return;
                }
                client_engine->postDeployProgress(
                    (float)i, std::string("Deploy progress = ") + std::to_string(i));
                static std::random_device sd;
                static std::minstd_rand linearRan(sd());
                static std::uniform_int_distribution<unsigned> round(1, 600);
                Thread::sleepMilli(round(linearRan));
            }
            client_engine->postDeployDone(true);
        });
    // deploy about 30s
    int reval = client_engine->exec(ClientEngine::CHECK_TERMINATE);
    DCUS_CLIENT_DESTROY();
    return reval;
}
