typedef struct {
    team_t      sessionTeam;
    team_t      respawnPointTeam;   // team we were on when respawnpoint was set
    int         spectatorTime;      // for determining next-in-line to play
    spectatorState_t    spectatorState;
    int         spectatorClient;    // for chasecam and follow mode
    int         wins, losses;       // tournament stats
    qboolean    teamLeader;         // true when this client is a team leader
    vec3_t      respawnPoint;
    int         bestTime;
    int         ignoreBitmap[2];    // MAX_CLIENTS = 64, so 2 ints needed
} clientSession_t;

typedef enum {
    VOTE_ABSTAIN,
    VOTE_YES,
    VOTE_NO
} voteState_t;

typedef struct {
    clientConnected_t   connected;
    usercmd_t   cmd;                // we would lose angles if not persistant
    qboolean    localClient;        // true if "ip" info key is "localhost"
    qboolean    initialSpawn;       // the first spawn should be at a cool location
    qboolean    predictItemPickup;  // based on cg_predictItems userinfo
    qboolean    pmoveFixed;         //
    char        netname[MAX_NETNAME];
    int         maxHealth;          // for handicapping
    int         enterTime;          // level.time the client entered the game
    playerTeamState_t teamState;    // status in teamplay games
    int         voteCount;          // to prevent people from constantly calling votes
    int         teamVoteCount;      // to prevent people from constantly calling votes
    qboolean    teamInfo;           // send team overlay updates?
    char        dfName[20];
#ifdef DF_1_91_27
    int         fastRespawn;
#endif
    voteState_t voteState;
} clientPersistant_t;