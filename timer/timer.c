void timer_run(gentity_t *ent, int msec)
{
    timerInfo_t *timer;

    timer = get_timer(ent->client);
    if (!timer) return;
    if (!ent->client || ent->client->ps.stats[STAT_HEALTH] < 1) {
        timer->event = TIMER_NO_EVENT;
        return;
    }
    if (timer->timer_running) {
        timer->time += msec;
        if (timer->event == TIMER_CHECKPOINT_EVENT) {
            ent->client->ps.stats[12] ^= 0x10; // TODO: unmagic
        }
        if (timer->event == TIMER_STOP_EVENT) {
            ent->client->ps.stats[12] ^= 8; // TODO: unmagic
            timer->total_timer_offset -= timer->timer_offset;
            timer->timer_running = qfalse;
            timer->one_ms_time = timer->time + (int)(timer->total_timer_offset + 0.5);
            ClientTimerStop(ent);
        }
    }
    if (timer->event == TIMER_START_EVENT) {
        reset_timer_but_not_its_event_and_timer_offset(ent);
        timer->timer_running = qtrue;
        timer->total_timer_offset += timer->timer_offset;
        ent->client->ps.stats[12] ^= 4;
    }
    timer->one_ms_time = timer->time + (int)(timer->total_timer_offset + 0.5);
    timer->event = TIMER_NO_EVENT;
    timer->timer_offset = 0.0;
}