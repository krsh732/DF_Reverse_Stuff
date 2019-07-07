typedef struct {
    // state (in / out)
    playerState_t    *ps;

    // command (in)
    usercmd_t    cmd;
    int            tracemask;            // collide against these types of surfaces
    qboolean    killWallBug
    int            debugLevel;            // if set, diagnostic output will be printed
    qboolean    noFootsteps;        // if the game is setup for no footsteps by the server
    qboolean    gauntletHit;        // true if a gauntlet attack would actually hit something

    int            framecount;

    // results (out)
    int            numtouch;
    int            touchents[MAXTOUCH];

    vec3_t        mins, maxs;            // bounding box size

    int            watertype;
    int            waterlevel;

    float        xyspeed;

    // for fixed msec Pmove
    int            pmove_fixed;
    int            pmove_msec;

    // callbacks to test the world
    // these will be different functions during game and cgame
    void        (*trace)( trace_t *results, const vec3_t start, vec3_t mins, vec3_t maxs, const vec3_t end, int passEntityNum, int contentMask );
    int            (*pointcontents)( const vec3_t point, int passEntityNum );
} pmove_t;