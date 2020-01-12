typedef enum {
	TIMER_NO_EVENT,
	TIMER_START_EVENT,
	TIMER_STOP_EVENT,
	TIMER_CHECKPOINT_EVENT
} timerEvent_t;

typedef struct {
    int time;
    timerEvent_t event;
    qboolean timer_running;
    int checkpoint_bitmap;
    int num_checkpoints;
} timerInfo_t;