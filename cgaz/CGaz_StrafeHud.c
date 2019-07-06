void CGaz_StrafeHud_Draw(float opacity, int ypos) {
    vec4_t blue = { 0.5, 0.5, 1.0, 1.0 };

    if ( !cg.snap || cg.hyperspace ) {
        return;
    }

    blue[3] = opacity;

    var_78 = tan( DEG2RAD(cg.refdef.fov_x / 2.0f) );
    yaw = cg.predictedPlayerState.viewangles[1];

    // if wasd or some drawing related var or something?
    if ( (cg.predictedPlayerState.stats[13] & 0x1b) || !global_11c954 ) {
        yaw = yaw + 45.0 * cg.predictedPlayerState.movementDir;
    }

    if ( fabs( cg.predictedPlayerState.velocity[0] ) + fabs( cg.predictedPlayerState.velocity[1] ) <= 0 ) {
        return;
    }

    vec3_t xyvel;
    xyvel[0] = cg.predictedPlayerState.velocity[0];
    xyvel[1] = cg.predictedPlayerState.velocity[1];
    xyvel[2] = 0;
    speed = VectorLength(xyvel);
    velocity_angle = RAD2DEG( atan2( cg.predictedPlayerState.velocity[1], cg.predictedPlayerState.velocity[0] ) );

    accel = cg.snap->ps.speed * (pmove_msec.value / 1000.0f);
    if ( cg.predictedPlayerState.groundEntityNum != ENTITYNUM_NONE ) {
        accel = 10.0f * accel; // hard coded for vq3
        speed = speed * (1.0f - (6.0f * (pmove_msec.value / 1000.0f))) // friction
    }

    if ( speed <= cg.snap->ps.speed - accel ) {
        return;
    }

    vec4_t colors[] = {
        { 1.0,  1.0,  0.0,  1.0 }, // yellow
        { 0.0,  0.25, 0.25, 1.0 }, // cyan
        { 0.0,  1.0,  0.0,  1.0 }, // green
        { 0.25, 0.25, 0.25, 1.0 }, // grey
        { 0.25, 0.25, 0.25, 1.0 }, // grey
        { 0.0,  1.0,  0.0,  1.0 }, // green
        { 0.0,  0.25, 0.25, 1.0 }, // cyan
        { 1.0,  1.0,  0.0,  1.0 }  // yellow
    }

    colors[0][3] *= opacity;
    colors[1][3] *= opacity;
    colors[2][3] *= opacity;
    colors[3][3] *= opacity;
    colors[4][3] *= opacity;
    colors[5][3] *= opacity;
    colors[6][3] *= opacity;
    colors[7][3] *= opacity;

    var_1ac = RAD2DEG( acos( (-0.5f * accel) / speed ) );
    opt_angle = RAD2DEG( acos( (cg.snap->ps.speed - accel) / speed ) );

    min_angle = 0;
    if ( speed > cg.snap->ps.speed ) {
        min_angle = RAD2DEG( acos( cg.snap->ps.speed / speed ) );
    }

    angles[0] = velocity_angle - var_1ac;
    angles[1] = velocity_angle - 90.0f;
    angles[2] = velocity_angle - opt_angle;
    angles[3] = velocity_angle - min_angle;
    angles[4] = velocity_angle;
    angles[5] = velocity_angle + min_angle;
    angles[6] = velocity_angle + opt_angle;
    angles[7] = velocity_angle + 90.0f;
    angles[8] = velocity_angle + var_1ac;

    for(i = 0; i < 8; i++) {
        angle_a = AngleSubtract( yaw, angles[i] );
        angle_b = AngleSubtract( yaw, angles[i+1] );

        // clip
        if ( fabs( angle_a ) > (cg.refdef.fov_x / 2.0f) ) {
            angle_a = (cg.refdef.fov_x / 2.0f) * (angle_a < 0.0f ? -1.0f : 1.0f);
        }

        // clip
        if ( fabs( angle_b ) > (cg.refdef.fov_x / 2.0f) ) {
            angle_b = (cg.refdef.fov_x / 2.0f) * (angle_b < 0.0f ? -1.0f : 1.0f);
        }

        x_a = (320.0f * tan( DEG2RAD(angle_a) )) / var_78;
        x_b = (320.0f * tan( DEG2RAD(angle_b) )) / var_78;
        if ( x_a >= x_b ) {
            CG_FillRect( x_b + 320.0f, ypos, x_a - x_b, 16.0f, colors[i] );
            if ( i == 4 ) {
                CG_FillRect( x_a + 320.0f - 1.0f, ypos + 2.0f, 2.0f, 12.0f, blue ); // little blue bar in the middle
            }
        }
    }
}