def shl32(x, n):
    return (x << n) & 0xffffffff

def shr32(x, n):
    return (x & 0xffffffff) >> n

def get_time(ps, snap_serverTime):
    time = shl32(ps.stats[7], 0x10) | (ps.stats[8] & 0xffff)

    if time == 0:
        return 0

    time ^= abs(int(math.floor(ps.origin[0]))) & 0xffff
    time ^= shl32(abs(int(math.floor(ps.velocity[0]))), 0x10)
    time ^= ps.stats[0] & 0xff if ps.stats[0] > 0 else 150
    time ^= shl32(ps.movementDir & 0xf, 0x1c)

    # if time was byte array (least significant at time[0]):
    # time[3] ^= time[2]
    # time[2] ^= time[1]
    # time[1] ^= time[0]
    # time[0] unchanged
    for i in range(0x18, 0, -8):
        temp = (shr32(time, i) ^ shr32(time, i-8)) & 0xff
        time = (time & ~shl32(0xff, i)) | shl32(temp, i)

    local1c = shl32(snap_serverTime, 2)
    df_ver = 19124
    map_type = 24 # global_11cdc8, not sure why i called this map_type
    local1c += shl32(df_ver + map_type, 8)
    local1c ^= shl32(snap_serverTime, 0x18)
    time ^= local1c
    local1c = shr32(time, 0x1c) # time[28:32]
    local1c |= shl32(~local1c, 4) & 0xff
    local1c |= shl32(local1c, 8)
    local1c |= shl32(local1c, 0x10)
    time ^= local1c
    local1c = shr32(time, 0x16) & 0x3f # time[22:28]
    time &= 0x3fffff

    # local20 = time[0:6] + time[6:12] + time[12:18] ...
    local20 = 0
    for l in range(3):
        local20 += shr32(time, 6*l) & 0x3f

    # ... + time[18:22]
    local20 += shr32(time, 0x12) & 0xf
    if local1c != local20 & 0x3f:
       print "bad checksum"

    return time