#define PMF_PROMODE 0x8000
// needs to be moved to statIndex_t one day
#define STAT_JUMPTIME 10

void PM_AddEvent( int newEvent ) {
    BG_AddPredictableEventToPlayerstate( newEvent, 0, pm->ps );
}

void PM_AddTouchEnt( int entityNum ) {
    int     i;

    if ( entityNum == ENTITYNUM_WORLD ) {
        return;
    }
    if ( pm->numtouch == MAXTOUCH ) {
        return;
    }

    // see if it is already added
    for ( i = 0 ; i < pm->numtouch ; i++ ) {
        if ( pm->touchents[ i ] == entityNum ) {
            return;
        }
    }

    // add it
    pm->touchents[pm->numtouch] = entityNum;
    pm->numtouch++;
}

static void PM_StartTorsoAnim( int anim ) {
    if ( pm->ps->pm_type >= PM_DEAD ) {
        return;
    }
    pm->ps->torsoAnim = ( ( pm->ps->torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT )
        | anim;
}

static void PM_StartLegsAnim( int anim ) {
    if ( pm->ps->pm_type >= PM_DEAD ) {
        return;
    }
    if ( pm->ps->legsTimer > 0 ) {
        return;     // a high priority animation is running
    }
    pm->ps->legsAnim = ( ( pm->ps->legsAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT )
        | anim;
}

static void PM_ContinueLegsAnim( int anim ) {
    if ( ( pm->ps->legsAnim & ~ANIM_TOGGLEBIT ) == anim ) {
        return;
    }
    if ( pm->ps->legsTimer > 0 ) {
        return;     // a high priority animation is running
    }
    PM_StartLegsAnim( anim );
}

static void PM_ContinueTorsoAnim( int anim ) {
    if ( ( pm->ps->torsoAnim & ~ANIM_TOGGLEBIT ) == anim ) {
        return;
    }
    if ( pm->ps->torsoTimer > 0 ) {
        return;     // a high priority animation is running
    }
    PM_StartTorsoAnim( anim );
}

static void PM_ForceLegsAnim( int anim ) {
    pm->ps->legsTimer = 0;
    PM_StartLegsAnim( anim );
}

void PM_ClipVelocity( vec3_t in, vec3_t normal, vec3_t out, float overbounce ) {
    float   backoff;
    float   change;
    int     i;

    backoff = DotProduct (in, normal);

    if ( backoff < 0 ) {
        backoff *= overbounce;
    } else {
        backoff /= overbounce;
    }

    for ( i=0 ; i<3 ; i++ ) {
        change = normal[i]*backoff;
        out[i] = in[i] - change;
    }
}

static void PM_Friction( void ) {
    vec3_t  vec;
    float   *vel;
    float   speed, newspeed, control;
    float   drop;

    vel = pm->ps->velocity;

    VectorCopy( vel, vec );
    if ( pml.walking ) {
        vec[2] = 0; // ignore slope movement
    }

    speed = VectorLength(vec);
    if (speed < 1) {
        vel[0] = 0;
        vel[1] = 0;     // allow sinking underwater
        // FIXME: still have z friction underwater?
        return;
    }

    drop = 0;

    // apply ground friction
    if ( pm->waterlevel <= 1 ) {
        if ( pml.walking && !(pml.groundTrace.surfaceFlags & SURF_SLICK) ) {
            // if getting knocked back, no friction
            if ( ! (pm->ps->pm_flags & PMF_TIME_KNOCKBACK) ) {
                control = speed < pm_stopspeed ? pm_stopspeed : speed;
                drop += control*pm_friction*pml.frametime;
            }
        }
    }

    // apply water friction even if just wading
    if ( pm->waterlevel ) {
        if ( pm->ps->pm_flags & PMF_PROMODE ) {
            drop += 0.5*speed*pm->waterlevel*pml.frametime;
        } else {
            drop += speed*pm_waterfriction*pm->waterlevel*pml.frametime;
        }
    }

    // apply flying friction
    if ( pm->ps->powerups[PW_FLIGHT]) {
        drop += speed*pm_flightfriction*pml.frametime;
    }

    if ( pm->ps->pm_type == PM_SPECTATOR) {
        drop += speed*pm_spectatorfriction*pml.frametime;
    }

    // scale the velocity
    newspeed = speed - drop;
    if (newspeed < 0) {
        newspeed = 0;
    }
    newspeed /= speed;

    vel[0] = vel[0] * newspeed;
    vel[1] = vel[1] * newspeed;
    vel[2] = vel[2] * newspeed;
}

static void PM_Accelerate( vec3_t wishdir, float wishspeed, float accel ) {
    // q2 style
    int         i;
    float       addspeed, accelspeed, currentspeed;

    currentspeed = DotProduct (pm->ps->velocity, wishdir);
    addspeed = wishspeed - currentspeed;
    if (addspeed <= 0) {
        return;
    }
    accelspeed = accel*pml.frametime*wishspeed;
    if (accelspeed > addspeed) {
        accelspeed = addspeed;
    }

    for (i=0 ; i<3 ; i++) {
        pm->ps->velocity[i] += accelspeed*wishdir[i];
    }
}

static float PM_CmdScale( usercmd_t *cmd ) {
    int     max;
    float   total;
    float   scale;

    max = abs( cmd->forwardmove );
    if ( abs( cmd->rightmove ) > max ) {
        max = abs( cmd->rightmove );
    }
    if ( abs( cmd->upmove ) > max ) {
        max = abs( cmd->upmove );
    }
    if ( !max ) {
        return 0;
    }

    total = sqrt( cmd->forwardmove * cmd->forwardmove
        + cmd->rightmove * cmd->rightmove + cmd->upmove * cmd->upmove );
    scale = (float)pm->ps->speed * max / ( 127.0 * total );

    return scale;
}

static void PM_SetMovementDir( void ) {
    if ( pm->cmd.forwardmove || pm->cmd.rightmove ) {
        if ( pm->cmd.rightmove == 0 && pm->cmd.forwardmove > 0 ) {
            pm->ps->movementDir = 0;
        } else if ( pm->cmd.rightmove < 0 && pm->cmd.forwardmove > 0 ) {
            pm->ps->movementDir = 1;
        } else if ( pm->cmd.rightmove < 0 && pm->cmd.forwardmove == 0 ) {
            pm->ps->movementDir = 2;
        } else if ( pm->cmd.rightmove < 0 && pm->cmd.forwardmove < 0 ) {
            pm->ps->movementDir = 3;
        } else if ( pm->cmd.rightmove == 0 && pm->cmd.forwardmove < 0 ) {
            pm->ps->movementDir = 4;
        } else if ( pm->cmd.rightmove > 0 && pm->cmd.forwardmove < 0 ) {
            pm->ps->movementDir = 5;
        } else if ( pm->cmd.rightmove > 0 && pm->cmd.forwardmove == 0 ) {
            pm->ps->movementDir = 6;
        } else if ( pm->cmd.rightmove > 0 && pm->cmd.forwardmove > 0 ) {
            pm->ps->movementDir = 7;
        }
    } else {
        // if they aren't actively going directly sideways,
        // change the animation to the diagonal so they
        // don't stop too crooked
        if ( pm->ps->movementDir == 2 ) {
            pm->ps->movementDir = 1;
        } else if ( pm->ps->movementDir == 6 ) {
            pm->ps->movementDir = 7;
        }
    }
}

static qboolean PM_CheckJump( void ) {
    if ( pm->ps->pm_flags & PMF_RESPAWNED ) {
        return qfalse;      // don't allow jump until all buttons are up
    }

    if ( pm->cmd.upmove < 10 ) {
        // not holding jump
        return qfalse;
    }

    // must wait for jump to be released
    if ( pm->ps->pm_flags & PMF_JUMP_HELD ) {
        // clear upmove so cmdscale doesn't lower running speed
        pm->cmd.upmove = 0;
        return qfalse;
    }

    pml.groundPlane = qfalse;       // jumping away
    pml.walking = qfalse;
    pm->ps->pm_flags |= PMF_JUMP_HELD;
    pm->ps->groundEntityNum = ENTITYNUM_NONE;

    if ( pm->ps->velocity[2] <= 0 || !(pm->ps->pm_flags & PMF_PROMODE)) {
        pm->ps->velocity[2] = JUMP_VELOCITY;
    } else {
        pm->ps->velocity[2] += JUMP_VELOCITY;
    }

    if ( pm->ps->pm_flags & PMF_PROMODE ) {
        if ( pm->ps->stats[STAT_JUMPTIME] > 0 ) {
            pm->ps->velocity[2] += 100.0f;
            // what do I call this?
            pm->ps->stats[STAT_JUMPTIME + 1] = 1;
        }
        pm->ps->stats[STAT_JUMPTIME] = 400
    }

    PM_AddEvent( EV_JUMP );

    if ( pm->cmd.forwardmove >= 0 ) {
        PM_ForceLegsAnim( LEGS_JUMP )
        pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
    } else {
        PM_ForceLegsAnim( LEGS_JUMPB )
        pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
    }

    return qtrue;
}

static qboolean PM_CheckWaterJump( void ) {
    vec3_t  spot;
    int     cont;
    vec3_t  flatforward;

    if (pm->ps->pm_time) {
        return qfalse;
    }

    // check for water jump
    if ( pm->waterlevel != 2 ) {
        return qfalse;
    }

    flatforward[0] = pml.forward[0];
    flatforward[1] = pml.forward[1];
    flatforward[2] = 0;
    VectorNormalize (flatforward);

    VectorMA (pm->ps->origin, 30, flatforward, spot);
    spot[2] += 4;
    cont = pm->pointcontents (spot, pm->ps->clientNum );
    if ( !(cont & CONTENTS_SOLID) ) {
        return qfalse;
    }

    spot[2] += 16;
    cont = pm->pointcontents (spot, pm->ps->clientNum );
    if ( cont ) {
        return qfalse;
    }

    // jump out of water
    VectorScale (pml.forward, 200, pm->ps->velocity);
    pm->ps->velocity[2] = 350;

    pm->ps->pm_flags |= PMF_TIME_WATERJUMP;
    pm->ps->pm_time = 2000;

    return qtrue;
}

static void PM_WaterJumpMove( void ) {
    // waterjump has no control, but falls

    PM_StepSlideMove( qtrue );

    pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
    if (pm->ps->velocity[2] < 0) {
        // cancel as soon as we are falling down again
        pm->ps->pm_flags &= ~PMF_ALL_TIMES;
        pm->ps->pm_time = 0;
    }
}

static void PM_WaterMove( void ) {
    int     i;
    vec3_t  wishvel;
    float   wishspeed;
    vec3_t  wishdir;
    float   scale;
    float   vel;

    if ( PM_CheckWaterJump() ) {
        PM_WaterJumpMove();
        return;
    }

    PM_Friction ();

    scale = PM_CmdScale( &pm->cmd );
    //
    // user intentions
    //
    if ( !scale ) {
        wishvel[0] = 0;
        wishvel[1] = 0;
        wishvel[2] = -60;       // sink towards bottom
    } else {
        for (i=0 ; i<3 ; i++)
            wishvel[i] = scale * pml.forward[i]*pm->cmd.forwardmove + scale * pml.right[i]*pm->cmd.rightmove;

        wishvel[2] += scale * pm->cmd.upmove;
    }

    VectorCopy (wishvel, wishdir);
    wishspeed = VectorNormalize(wishdir);

    if(pm->ps->pm_flags & PMF_PROMODE)
    {
        if(pm->waterlevel == 1) {
            if(wishspeed > (0.585 * pm->ps->speed)) {
                wishspeed = 0.585 * pm->ps->speed;
            }
        } else if(wishspeed > (0.54 * pm->ps->speed)) {
            wishspeed = 0.54 * pm->ps->speed;
        }

        PM_Accelerate (wishdir, wishspeed, 5.0f);
    }
    else
    {
        if ( wishspeed > pm->ps->speed * pm_swimScale ) {
            wishspeed = pm->ps->speed * pm_swimScale;
        }

        PM_Accelerate (wishdir, wishspeed, pm_wateraccelerate);
    }

    // make sure we can go up slopes easily under water
    if ( pml.groundPlane && DotProduct( pm->ps->velocity, pml.groundTrace.plane.normal ) < 0 ) {
        vel = VectorLength(pm->ps->velocity);
        // slide along the ground plane
        PM_ClipVelocity (pm->ps->velocity, pml.groundTrace.plane.normal,
            pm->ps->velocity, OVERCLIP );

        VectorNormalize(pm->ps->velocity);
        VectorScale(pm->ps->velocity, vel, pm->ps->velocity);
    }

    PM_SlideMove( qfalse );
}

static void PM_FlyMove( void ) {
    int     i;
    vec3_t  wishvel;
    float   wishspeed;
    vec3_t  wishdir;
    float   scale;

    // normal slowdown
    PM_Friction ();

    scale = PM_CmdScale( &pm->cmd );
    //
    // user intentions
    //
    if ( !scale ) {
        wishvel[0] = 0;
        wishvel[1] = 0;
        wishvel[2] = 0;
    } else {
        for (i=0 ; i<3 ; i++) {
            wishvel[i] = scale * pml.forward[i]*pm->cmd.forwardmove + scale * pml.right[i]*pm->cmd.rightmove;
        }

        wishvel[2] += scale * pm->cmd.upmove;
    }

    VectorCopy (wishvel, wishdir);
    wishspeed = VectorNormalize(wishdir);

    PM_Accelerate (wishdir, wishspeed, pm_flyaccelerate);

    PM_StepSlideMove( qfalse );
}

static void PM_AirControl(pmove_t *pm, vec3_t wishdir, float wishspeed) {
    float zspeed, speed, dot, k;

    if ( (pm->ps->movementDir && pm->ps->movementDir != 4) || wishspeed == 0.0 ) {
        return;
    }

    zspeed = pm->ps->velocity[2];
    pm->ps->velocity[2] = 0;
    speed = VectorNormalize( pm->ps->velocity );
    dot = DotProduct( pm->ps->velocity, wishdir );
    if ( dot > 0 ) {
        k = 32*150*dot*dot*pml.frametime;
        pm->ps->velocity[0] = pm->ps->velocity[0]*speed + wishdir[0]*k;
        pm->ps->velocity[1] = pm->ps->velocity[1]*speed + wishdir[1]*k;
        VectorNormalize( pm->ps->velocity );
    }

    pm->ps->velocity[0] *= speed;
    pm->ps->velocity[1] *= speed;
    pm->ps->velocity[2] = zspeed;
}

static void PM_AirMove( void ) {
    int         i;
    vec3_t      wishvel;
    float       fmove, smove;
    vec3_t      wishdir;
    float       wishspeed;
    float       scale;
    usercmd_t   cmd;
    float       cpmairaccel;

    PM_Friction();

    fmove = pm->cmd.forwardmove;
    smove = pm->cmd.rightmove;

    cmd = pm->cmd;
    scale = PM_CmdScale( &cmd );

    // set the movementDir so clients can rotate the legs for strafing
    PM_SetMovementDir();

    // project moves down to flat plane
    pml.forward[2] = 0;
    pml.right[2] = 0;
    VectorNormalize (pml.forward);
    VectorNormalize (pml.right);

    for ( i = 0 ; i < 2 ; i++ ) {
        wishvel[i] = pml.forward[i]*fmove + pml.right[i]*smove;
    }
    wishvel[2] = 0;

    VectorCopy (wishvel, wishdir);
    wishspeed = VectorNormalize(wishdir);
    wishspeed *= scale;

    if ( (pm->ps->pm_flags & PMF_PROMODE) ) {
        cpmairaccel = pm_airaccelerate;
        if ( DotProduct(pm->ps->velocity, wishdir) < 0 ) {
            cpmairaccel = 2.5f;
        }
        if ( pm->ps->movementDir == 2 || pm->ps->movementDir == 6 ) {
            if ( wishspeed > 30.0f ) {
                wishspeed = 30.0f;
            }
            cpmairaccel = 70.0f;
        }
        PM_Accelerate( wishdir, wishspeed, cpmairaccel )
        PM_AirControl( pm, wishdir, wishspeed )
    } else {
        PM_Accelerate( wishdir, wishspeed, pm_airaccelerate )
    }

    // we may have a ground plane that is very steep, even
    // though we don't have a groundentity
    // slide along the steep plane
    if ( pml.groundPlane ) {
        PM_ClipVelocity (pm->ps->velocity, pml.groundTrace.plane.normal,
            pm->ps->velocity, OVERCLIP );
    }

    PM_StepSlideMove ( qtrue );
}