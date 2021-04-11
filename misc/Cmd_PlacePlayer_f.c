void Cmd_PlacePlayer_f(gentity_t *ent)
{
    const char *s;
    int i;
    char buf[MAX_TOKEN_CHARS];
    float angles[3];
    float origin[3];
    float velocity[3];

    if (!get_cheats_enabled()) {
        s = va("print \"Cheats are not enabled on this server.\n\"");
        trap_SendServerCommand(ent - g_entities, s);
        return;
    }
    if (trap_Argc() != 10) {
        s = va("print \"Usage: placeplayer x y z p y r v0 v1 v2\n\"");
        trap_SendServerCommand(ent - g_entities, s);
        return;
    }
    origin[2] = 0.0;
    origin[1] = 0.0;
    origin[0] = 0.0;
    angles[2] = 0.0;
    angles[1] = 0.0;
    angles[0] = 0.0;
    velocity[2] = 0.0;
    velocity[1] = 0.0;
    velocity[0] = 0.0;
    for (i = 0; i < 3; i++) {
        trap_Argv(i + 1, buf, MAX_TOKEN_CHARS);
        origin[i] = atof(buf);
    }
    for (i = 0; i < 3; i++) {
        trap_Argv(i + 4, buf, MAX_TOKEN_CHARS);
        angles[i] = atof(buf);
        angles[2] = 0.0;
    }
    for (i = 0; i < 3; i++) {
        trap_Argv(i + 7, buf, MAX_TOKEN_CHARS);
        velocity[i] = atof(buf);
    }
    placeplayer_teleport_44af(ent, origin, angles, velocity);
    s = va("print \"^3Restored:^7 ( %.3f %.3f %.3f ) %.3f %.3f ( %i %i %i )\n\"",
           origin[0], origin[1], origin[2], angles[0], angles[1], (int)velocity[0],
           (int)velocity[1], (int)velocity[2]);
    trap_SendServerCommand(ent - g_entities, s);
}