void CG_SavePos_f(void)
{
    vec3_t velocity;
    vec3_t origin;
    vec3_t viewangles;
    char saveposname[MAX_TOKEN_CHARS];
    char option_str[MAX_TOKEN_CHARS];
    int option;
    char set_saveposname_cmd[MAX_TOKEN_CHARS];
    char placeplayer_cmd[MAX_TOKEN_CHARS];

    trap_Argv(1, option_str, sizeof(option_str));
    option = atoi(option_str);
    if (!Q_stricmp(option_str, "?")) {
        CG_Printf("\n");
        CG_Printf("^1savepos: ^3Allows saving and restoring player position, angles, and velocity.\n");
        CG_Printf("^5Usage:   ^2savepos [ <option> ]\n");
        CG_Printf("^5Example: ^2see savepos documentation for details.\n");
        CG_Printf("\n");
    }
    trap_Cvar_VariableStringBuffer("saveposname", saveposname, sizeof(saveposname));
    if (!strlen(saveposname)) {
        CG_Printf("%s is empty. use: set %s <myposname> <options>\n", "saveposname",
                  "saveposname");
        return;
    }
    memcpy(origin, cg.snap->ps.origin, sizeof(origin));
    memcpy(velocity, cg.snap->ps.velocity, sizeof(velocity));
    memcpy(viewangles, cg.snap->ps.viewangles, sizeof(viewangles));
    if (option == 1) {
        velocity[2] = 0.0;
        velocity[1] = 0.0;
        velocity[0] = 0.0;
    } else if (option == 2) {
        velocity[2] = 0.0;
        velocity[1] = 0.0;
        velocity[0] = 0.0;
        viewangles[0] = 0.0;
    }
    sprintf(placeplayer_cmd, "placeplayer %f %f %f %f %f %f %f %f %f", origin[0], origin[1], origin[2],
            viewangles[0], viewangles[1], viewangles[2], velocity[0], velocity[1], velocity[2]);
    sprintf(set_saveposname_cmd, "set %s %s\n", saveposname, placeplayer_cmd);
    trap_SendConsoleCommand(set_saveposname_cmd);
    CG_Printf("^2\"%s\" Saved:^7 ( %.3f %.3f %.3f ) %.3f %.3f ( %i %i %i )\n", saveposname,
              origin[0], origin[1], origin[2], viewangles[0], viewangles[1], (int)(float)velocity[0],
              (int)(float)velocity[1], (int)(float)velocity[2]);
}

