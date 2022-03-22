uint8 MR_UNKNOWN = 0
uint8 MR_IDLE = 1
uint8 MR_PENDING = 2
uint8 MR_READY = 3
uint8 MR_DOWNLOAD = 4
uint8 MR_VERIFY = 5
uint8 MR_DISTRUBUTE = 6
uint8 MR_WAIT = 7
uint8 MR_DEPLOY = 8
uint8 MR_CANCEL = 9
uint8 MR_DOWNLOAD_ASK = 10
uint8 MR_DEPLOY_ASK = 11
uint8 MR_CANCEL_ASK = 12
uint8 MR_RESUME_ASK = 13
uint8 MR_DONE_ASK = 14
uint8 MR_ERROR_ASK = 15
uint32 id
uint8 state
uint8 last
bool active
int32 error
float32 step
float32 progress
string message
Detail[] details
