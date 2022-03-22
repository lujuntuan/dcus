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

#include <dcus/WorkerEngine>
#include <random>

using namespace DCus;

int main(int argc, char* argv[])
{
    DCUS_WORKER_CREATE(argc, argv);
    // worker_engine->setMeta(Data({ { "max_deploy_time", 8000 } }));
    worker_engine->setAttribute(Data({ { "hw_addr", "abcdefg" } }));
    worker_engine->setVersion("1.0.0");
    worker_engine->subscibeDeploy(
        [](const std::string& dir, const FilePaths filePaths) {
            for (int i = 0; i < 100; i++) {
                if (worker_engine->hasStopAction()) {
                    return;
                }
                if (worker_engine->hasCancelAction()) {
                    worker_engine->postCancelDone(true);
                    return;
                }
                worker_engine->postDeployProgress(
                    (float)i, std::string("Deploy progress = ") + std::to_string(i));
                static std::random_device sd;
                static std::minstd_rand linearRan(sd());
                static std::uniform_int_distribution<unsigned> round(1, 600);
                Thread::sleepMilli(round(linearRan));
            }
            worker_engine->postDeployDone(true);
        });
    // deploy about 30s
    int reval = worker_engine->exec(WorkerEngine::CHECK_TERMINATE);
    DCUS_WORKER_DESTROY();
    return reval;
}
