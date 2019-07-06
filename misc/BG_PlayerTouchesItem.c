qboolean BG_PlayerTouchesItem( playerState_t *ps, entityState_t *item, int atTime )
{
    vec3_t origin;

    BG_EvaluateTrajectory( &item->pos, atTime, origin );

    if( ps->origin[0] - origin[0] > 44.0f
        || ps->origin[0] - origin[0] < -50.0f
        || ps->origin[1] - origin[1] > 36.0f
        || ps->origin[1] - origin[1] < -36.0f ) {
        return qfalse;
    }
    if(cpm) {
        if( ps->origin[2] - origin[2] > 66.0f
            || ps->origin[2] - origin[2] < -36.0f ) {
            return qfalse;
        }
    }
    else {
        if(ps->origin[2] - origin[2] < -36.0f
            || ps->origin[2] - origin[2] > 36.0f ) {
            return qfalse;
        }
    }

    return qtrue;
}