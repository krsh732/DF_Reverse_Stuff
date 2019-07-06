//  ent->damage lower 8 bits is the number of targets to trigger randomly (count)
//  ent->damage upper 8 bits is the total number of targets
//  ent->target is <delay01>\0<targetname01>\0<delay02>\0<targetname02>\0...
//  ent->count is length of ent->target
//  ent->health is how many targets still need to be triggered before the multimanager can be used again

void Think_Multimanager_Target_Delay( gentity_t *ent ) {
    if ( ent->count != -1 )
        G_UseTargets( ent, ent->activator );

    ent->nextthink = level.time + FRAMETIME;
    ent->think = G_FreeEntity;
    ent->parent->health--;
    if ( ent->parent->health == 0 )
        ent->parent->activator = NULL;
}

// randomly chooses between 0 and 40 (based on `num_random_targets`) indices of `targets` and sets them to qtrue
void choose_targets( qboolean *targets, int num_random_targets, int num_targets ) {
    int remaining[40];
    int num_remaining;
    int i, j, k;
    int choice;

    if ( num_targets > 40 )
        num_targets = 40;

    if ( num_random_targets > num_targets )
        num_random_targets = num_targets;

    if ( num_random_targets == num_targets ) {
        for( i = 0; i < num_targets; i++ )
            targets[i] = 1;
        return;
    }

    for( i = 0; i < num_targets; i++ ) {
        remaining[i] = i;
        targets[i] = 0;
    }

    num_remaining = num_targets;

    // `j--;` was actually between `targets[remaining[i]] = 1;` and `num_remaining--;`
    // would q3lcc do that?
    // choice reuses i's memory
    for( j = num_random_targets; j > 0; j-- ) {
        choice = rand() % num_remaining;
        targets[remaining[choice]] = 1;
        num_remaining--;

        // remove chosen target from list
        for( k = choice; k < num_remaining; k++ )
            remaining[k] = remaining[k+1];
    }
}

void Use_Target_Multimanager( gentity_t *ent, gentity_t *other, gentity_t *activator ) {
    gentity_t *target, *delay;
    char *target_string, *target_name;
    qboolean targets[40];
    int target_number = 0;
    float wait;

    if ( ent->spawnflags & 1 ) {
        if ( activator->client && activator->client->sess.sessionTeam != TEAM_RED )
            return;
    }

    if ( ent->spawnflags & 2 ) {
        if ( activator->client && activator->client->sess.sessionTeam != TEAM_BLUE )
            return;
    }

    if ( ent->health )
        return;

    ent->activator = activator;
    ent->health = ent->damage & 0xff;
    choose_targets( targets, ent->health, ent->damage >> 8 );
    target_string = ent->target;

    while ( target_string < ent->count + ent->target ) {
        if ( !strcmp( target_string, "-1" ) ) {
            if ( ent->spawnflags & 8 ) {
                wait = ent->wait * (target_number + 1);
            } else {
                wait = ent->wait;
            }
        } else {
            wait = atof( target_string );
            if ( ent->spawnflags & 8 ) {
                wait += ent->wait * (target_number + 1);
            } else if ( ent->spawnflags & 4 ) {
                wait += ent->wait;
            }
        }

        target_string += strlen( target_string ) + 1;
        target_name = target_string;
        target_string += strlen( target_name ) + 1;

        if ( !targets[target_number++] )
            continue;

        target = G_PickTarget( target_name );
        if ( !target || !target->use ) {
            G_Printf( "Invalid target was specified in multimanager\n" );
            continue;
        }

        if ( wait <= 0.001f ) {
            ent->health--;
            target->use( target, ent, activator );
            continue;
        }

        delay = G_Spawn();
        delay->classname = "target_delay";
        delay->parent = ent;
        delay->target = target_name;
        delay->activator = activator;
        delay->think = Think_Multimanager_Target_Delay;
        delay->nextthink = level.time + 1000.0f * (wait + ent->random * crandom());
    }
}

void SP_target_multimanager( gentity_t *ent ) {
    char *p, *s;
    char key[9], target_string[1024];
    int i, len, target_len;
    int num_targets = 0;

    G_SpawnFloat( "random", "0", &ent->random );
    G_SpawnFloat( "wait", "1", &ent->wait );
    G_SpawnInt( "count", "-1", &ent->damage );

    if ( !ent->damage ) {
        G_Printf( "Warning: target_multimanager \"%s\" with count 0. No targets will be triggered!\n", ent->targetname ? ent->targetname : "unnamed" );
        ent->nextthink = level.time + FRAMETIME;
        ent->think = G_FreeEntity;
        return;
    }

    ent->health = 0;
    p = target_string;
    ent->count = 0;

    for( i = 0; i < 40; i++ ) {
        strcpy( key, "target" );
        key[6] = (i + 1) / 10 + '0';
        key[7] = (i + 1) % 10 + '0';
        key[8] = 0;

        if ( !G_SpawnString( key, "", &s ) )
            continue;

        if ( strlen( s ) == 0x0 ) {
            G_Printf( "Empty target-string for target%02d of multimanager \"%s\".\n", i + 1, ent->targetname );
            continue;
        }

        len = strlen( s ) + 1;
        if ( !strstr( s, ":" ) )
            len += 3;

        if ( ent->count + len > sizeof(target_string) ) {
            G_Printf( "Too much memory allocated for target_multimanager \"%s\"\n", ent->targetname );
            ent->nextthink = level.time + FRAMETIME;
            ent->think = G_FreeEntity;
            return;
        }

        // no delay was specified
        if ( !strstr( s, ":" ) )
            strcat( p, "-1:" );

        strcat( p, s );
        strcat( p, ":" );
        p += len;
        ent->count += len;

        num_targets++;
    }

    if ( !num_targets ) {
        ent->nextthink = level.time + FRAMETIME;
        ent->think = G_FreeEntity;
        return;
    }

    // if count wasn't set or was wrong then trigger all of them randomly
    if ( ent->damage < 0 || ent->damage > num_targets )
        ent->damage = num_targets;

    ent->damage |= num_targets << 8;

    ent->target = G_Alloc( ent->count );
    Q_strncpyz( ent->target, target_string, strlen( target_string ) );

    target_len = strlen( ent->target );

    for( i = 0; i < target_len; i++ ) {
        if ( ent->target[i] == ':' )
            ent->target[i] = 0;
    }

    ent->use = Use_Target_Multimanager;
}