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

#include "dcus/config.h"

#ifndef DCUS_SETTING_H
#define DCUS_SETTING_H

#define DCUS_USE_MESSAGE_HTTP
#define DCUS_USE_PULL_HTTP
#define DCUS_USE_DOWNLOAD_HTTP
#define DCUS_USE_DISTRIBUTE_HTTP
#define DCUS_USE_VERIFY_OPENSSL
#define DCUS_USE_PATCH_BSDIFF

#define DCUS_QUEUE_ID_SERVER 1
#define DCUS_QUEUE_ID_CLIENT 2
#define DCUS_QUEUE_ID_WEB 3

#define DCUS_WEB_USE_POLLING 1
#define DCUS_WEB_CHECK_INTERVAL 10000
#define DCUS_WEB_TRANSFER_INTERVAL 500
#define DCUS_WEB_TRANSFER_INTERVAL_MIN 50
#define DCUS_WEB_TIMEOUT 10000
#define DCUS_WEB_TRANSFER_TEST_TIME 0
#define DCUS_WEB_PULL_FUNC_NAME "dcus"

#define DCUS_MAX_PENDING_TIME 60000 * 5
#define DCUS_MAX_PENDING_TIME_FIRST 60000 * 60
#define DCUS_MAX_ASK_TIME 60000 * 5
#define DCUS_MAX_CANCEL_TIME 60000 * 60
#define DCUS_MAX_VERIFY_TIME 60000 * 5
#define DCUS_MAX_TRANSFER_TIME 60000 * 60 * 6
#define DCUS_MAX_DEPLOY_TIME 60000 * 60 * 6
#define DCUS_MAX_DEPLOY_TIME_CLIENT 60000 * 60
#define DCUS_MAX_DEPLOY_RESTART_TIME_CLIENT 60000 * 1

#define DCUS_PROCESS_DOMAIN_TIME 1000
#define DCUS_HEARTBEAT_TIME_OUT 6000
#define DCUS_MESSAGE_TOTAL_COUNT 100000
#define DCUS_DOWNLOAD_KEEP_FILE_COUNT 2
#define DCUS_RETRY_TIMES 2

#define DCUS_SERVER_NAME "dcus_server"

#endif // DCUS_SETTING_H
