uint8 WR_UNKNOWN = 0
uint8 WR_ERROR = 1
uint8 WR_IDLE = 2
uint8 WR_DOWNLOAD = 3
uint8 WR_VERIFY = 4
uint8 WR_PATCH = 5
uint8 WR_WAIT = 6
uint8 WR_DEPLOY = 7
uint8 ANS_UNKNOWN = 0
uint8 ANS_ACCEPT = 1
uint8 ANS_REFUSE = 2
uint8 ANS_OTHER = 3
string name
string guid
uint8 state
uint8 last
bool watcher
uint32 error
string version
string attribute
string meta
float32 progress
string message
uint8 answer
