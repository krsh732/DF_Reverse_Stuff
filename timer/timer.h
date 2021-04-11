typedef enum {
    TIMER_NO_EVENT,
    TIMER_START_EVENT,
    TIMER_STOP_EVENT,
    TIMER_CHECKPOINT_EVENT
} timerEvent_t;

#if 0
// DF 1.91.26 or earlier
typedef struct {
    int time;
    timerEvent_t event;
    qboolean timer_running;
    int checkpoint_bitmap;
    int num_checkpoints;
} timerInfo_t;
#endif

// DF 1.91.27
typedef struct {
    int time;
    int one_ms_time;
    // probably, I don't really care enough to make sure; part of new 1 ms thing
    float total_timer_offset;
    timerEvent_t event;
    // probably 0 most of the time and only ever used on start/stop events, but I don't care enough for now; part of new 1 ms thing
    float timer_offset;
    qboolean timer_running;
    int checkpoint_bitmap;
    int num_checkpoints;
} timerInfo_t;