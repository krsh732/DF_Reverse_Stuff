typedef struct {
    int      time;
    int      events; // verify and document more
    qboolean timer_running;
    int      checkpoint_bitmap;
    int      num_checkpoints;
} timeInfo_t;
