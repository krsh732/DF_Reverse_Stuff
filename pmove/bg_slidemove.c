void PM_FlatClipVelocity( vec3_t in, vec3_t normal, vec3_t out, float overbounce ) {
    float   backoff;
    float   change;
    int     i;

    backoff = DotProduct (in, normal);

    if ( backoff < 0 ) {
        backoff *= overbounce;
    } else {
        backoff /= overbounce;
    }

    for ( i=0 ; i<2 ; i++ ) {
        change = normal[i]*backoff;
        out[i] = in[i] - change;
    }
}

/*
==================
PM_StepSlideMove
==================
*/
void PM_StepSlideMove( qboolean gravity ) {
    vec3_t      start_o, start_v;
    vec3_t      down_o, down_v;
    trace_t     trace;
//  float       down_dist, up_dist;
//  vec3_t      delta, delta2;
    vec3_t      up, down;
    float       stepSize;

    VectorCopy (pm->ps->origin, start_o);
    VectorCopy (pm->ps->velocity, start_v);

    if ( PM_SlideMove( gravity ) == 0 ) {
        return;     // we got exactly where we wanted to go first try
    }

    VectorCopy(start_o, down);
    down[2] -= STEPSIZE;
    pm->trace (&trace, start_o, pm->mins, pm->maxs, down, pm->ps->clientNum, pm->tracemask);
    VectorSet(up, 0, 0, 1);
    // never step up when you still have up velocity
    if ( pm->ps->velocity[2] > 0 && (trace.fraction == 1.0 ||
            DotProduct(trace.plane.normal, up) < 0.7)) {
        if ( !(pm->ps->pm_flags & PMF_PROMODE) || pm->ps->velocity[2] > 370.0 || pm->stats[STAT_JUMPTIME] <= 0 ) {
            return;
        }
    }

    VectorCopy (pm->ps->origin, down_o);
    VectorCopy (pm->ps->velocity, down_v);

    VectorCopy (start_o, up);
    up[2] += STEPSIZE;

    // test the player position if they were a stepheight higher
    pm->trace (&trace, start_o, pm->mins, pm->maxs, up, pm->ps->clientNum, pm->tracemask);
    if ( trace.allsolid ) {
        if ( pm->debugLevel ) {
            Com_Printf("%i:bend can't step\n", c_pmove);
        }
        if ( pm->killwallbugs ) {
            VectorClear( pm->ps->velocity );
        }
        return;     // can't step up
    }

    stepSize = trace.endpos[2] - start_o[2];
    // try slidemove from this position
    VectorCopy (trace.endpos, pm->ps->origin);
    VectorCopy (start_v, pm->ps->velocity);

    PM_SlideMove( gravity );

    // push down the final amount
    VectorCopy (pm->ps->origin, down);
    down[2] -= stepSize;
    pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, down, pm->ps->clientNum, pm->tracemask);
    if ( !trace.allsolid ) {
        VectorCopy (trace.endpos, pm->ps->origin);
    }
    if ( trace.fraction < 1.0 ) {
        if ( pm->ps->pm_flags & PMF_PROMODE && pm->ps->stats[STAT_DJING] ) {
            PM_FlatClipVelocity( pm->ps->velocity, trace.plane.normal, pm->ps->velocity, OVERCLIP);
            return;
        }
        PM_ClipVelocity( pm->ps->velocity, trace.plane.normal, pm->ps->velocity, OVERCLIP );
    }

#if 0
    // if the down trace can trace back to the original position directly, don't step
    pm->trace( &trace, pm->ps->origin, pm->mins, pm->maxs, start_o, pm->ps->clientNum, pm->tracemask);
    if ( trace.fraction == 1.0 ) {
        // use the original move
        VectorCopy (down_o, pm->ps->origin);
        VectorCopy (down_v, pm->ps->velocity);
        if ( pm->debugLevel ) {
            Com_Printf("%i:bend\n", c_pmove);
        }
    } else
#endif
    {
        // use the step move
        float   delta;
        delta = pm->ps->origin[2] - start_o[2];
        if ( delta > 2 ) {
            if ( delta < 7 ) {
                PM_AddEvent( EV_STEP_4 );
            } else if ( delta < 11 ) {
                PM_AddEvent( EV_STEP_8 );
            } else if ( delta < 15 ) {
                PM_AddEvent( EV_STEP_12 );
            } else {
                PM_AddEvent( EV_STEP_16 );
            }
        }
        if ( pm->debugLevel ) {
            Com_Printf("%i:stepped\n", c_pmove);
        }
    }
}