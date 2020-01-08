void SP_trigger_push_velocity(gentity_t *ent)
{
  InitTrigger(ent);
  ent->r.svFlags &= ~SVF_NOCLIENT;
  G_SoundIndex("sound/world/jumppad.wav");
  ent->s.eType = ET_PUSH_TRIGGER;
  ent->touch = trigger_push_velocity_touch;
  ent->think = AimAtTarget;
  ent->nextthink = level.time + 100;
  ent->s.weapon = ent->spawnflags;
  ent->s.angles2[0] = ent->speed;
  ent->s.angles2[1] = ent->speed;
  ent->s.angles2[2] = ent->count;
  trap_LinkEntity(ent);
}

void trigger_push_velocity_touch(gentity_t *ent,gentity_t *other)
{
  if (other->client && (fc_bullshit? != 1)) {
    if (fc_bullshit? != 0) {
      G_Damage(other,ent,ent,0,0,9999,DAMAGE_NO_PROTECTION,MOD_TRIGGER_HURT);
    }
    BG_TouchVelocityJumpPad(&other->client->ps, &ent->s);
  }
  return;
}

void BG_TouchVelocityJumpPad(playerState_t *ps, entityState_t *jumppad)
{
  int spawnflags;
  vec3_t velocityXY;
  vec3_t velocityZ;
  vec3_t plus;
  vec3_t angles;
  vec3_t minus;
  float p;

  // spectators don't use jump pads
  if ( ps->pm_type != PM_NORMAL ) {
    return;
  }

  // flying characters don't hit bounce pads
  if ( ps->powerups[PW_FLIGHT] ) {
    return;
  }

  if (ps->jumppad_ent != jumppad->number) {
    vectoangles( jumppad->origin2, angles);
    p = fabs( AngleNormalize180( angles[PITCH] ) );
    if( p < 45 ) {
      effectNum = 0;
    } else {
      effectNum = 1;
    }
    BG_AddPredictableEventToPlayerstate( EV_JUMP_PAD, effectNum, ps );
  }
  spawnflags = jumppad->weapon;
  if(spawnflags) {
    if (spawnflags & 1) {
      velocityXY[0] = ps->velocity[0];
      velocityXY[1] = ps->velocity[1];
      velocityXY[2] = 0;
      VectorNormalize(velocityXY);
      VectorScale( velocityXY, jumppad->angles2[0], velocityXY );
    } else {
      velocityXY[0] = jumppad->origin2[0];
      velocityXY[1] = jumppad->origin2[1];
      velocityXY[2] = 0;
      if (spawnflags & 16) {
        VectorCopy(ps->velocity, plus);
        VectorCopy(ps->velocity, minus);
        plus[0] += velocityXY[0];
        plus[1] += velocityXY[1];
        minus[0] -= velocityXY[0];
        minus[1] -= velocityXY[1];
        if (VectorLength(plus) < VectorLength(minus)) {
          VectorNegate( velocityXY, velocityXY );
        }
      }
    }
    if (spawnflags & 4) {
      velocityZ[0] = 0;
      velocityZ[1] = 0;
      velocityZ[2] = ps->velocity[2];
      VectorNormalize(velocityZ);
      VectorScale( velocityZ, jumppad->angles2[2], velocityZ );
    } else {
      velocityZ[0] = 0;
      velocityZ[1] = 0;
      velocityZ[2] = jumppad->origin2[2];
      if (spawnflags & 0x20) {
        VectorCopy(ps->velocity, plus);
        VectorCopy(ps->velocity, minus);
        plus[2] = plus[2] + velocityZ[2];
        minus[2] = minus[2] - velocityZ[2];
        if (VectorLength(plus) < VectorLength(minus)) {
          VectorNegate( velocityZ, velocityZ );
        }
      }
    }
    if (spawnflags & 2) {
      if (ps->jumppad_ent != jumppad->number) {
        plus[0] = ps->velocity[0] + velocityXY[0];
        plus[1] = ps->velocity[1] + velocityXY[1];
        plus[2] = 0;
        float theta = DotProduct(plus, ps->velocity) / (VectorLength(plus) * VectorLength(ps->velocity));
        if ((spawnflags & 0x40) && theta < 0)  {
          ps->velocity[0] = 0;
          ps->velocity[1] = 0;
        } else {
          ps->velocity[0] = plus[0];
          ps->velocity[1] = plus[1];
        }
      }
    } else {
      ps->velocity[0] = velocityXY[0];
      ps->velocity[1] = velocityXY[1];
    }
    if (spawnflags & 8) {
      if (ps->jumppad_ent != jumppad->number) {
        plus[0] = 0;
        plus[1] = 0;
        plus[2] = ps->velocity[2] + velocityZ[2];
        float theta = DotProduct(plus, ps->velocity) / (VectorLength(plus) * VectorLength(ps->velocity));
        if ((spawnflags & 0x40) && theta < 0) {
          ps->velocity[2] = 0;
        } else {
          ps->velocity[2] = plus[2];
        }
      }
    } else {
      ps->velocity[2] = velocityZ[2];
    }
  } else {
    VectorCopy(jumppad->origin2, ps->velocity);
  }
  ps->jumppad_ent = jumppad->number;
  ps->jumppad_frame = ps->pmove_framecount;
}
