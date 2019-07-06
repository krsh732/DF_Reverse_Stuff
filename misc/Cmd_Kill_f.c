void Cmd_Kill_f( gentity_t *ent ) {
    if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
        return;
    }
    if ( ent->health <= 0 ) {
        return;
    }
    if ( level.time - ent->client->respawnTime < 700 ) {
        return;
    }

    ClientSpawn(ent); // epic :+1:
    ent->flags &= ~FL_GODMODE;
    ent->client->ps.stats[STAT_HEALTH] = ent->health = -999;
    player_die(ent, ent, ent, 100000, MOD_SUICIDE);
}