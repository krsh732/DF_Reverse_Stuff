void SP_target_fragsFilter(gentity_t *ent)
{
  char *s;

  ent->use = Use_Target_FragsFilter;
  G_SpawnInt("frags","1",&ent->count);
  if (!G_SpawnString("targetname","decoy",s) && (some_global_num_indepdendent_fragsFilter < 10)) {
    some_global_independent_fragsFilter_ent_pointer_array[some_global_num_indepdendent_fragsFilter] = ent;
    some_global_num_indepdendent_fragsFilter++;
    ent->s.generic1 = 1;
  }
  ent->think = Think_Target_FragsFilter;
  ent->nextthink = level.time + 400;
  return;
}

void Think_Target_FragsFilter(gentity_t *ent)
{
  gentity_t *t;

  t = NULL;
  while ((t = (gentity_t *)G_Find(t,FOFS(targetname),ent->target)) != NULL) {
    if (!strcmp(t->classname,"target_stopTimer")) {
      if ((ent->count < some_global_fraglimit_storing_int_thing) || (some_global_fraglimit_storing_int_thing == 0)) {
        some_global_fraglimit_storing_int_thing = ent->count;
      }
      some_global_fragsfilter_related_time_thing = level.time + 200;
    }
  }
  return;
}

void Use_Target_FragsFilter(gentity_t *ent, gentity_t *other,gentity_t *activator)
{
  char *text;

  if (activator->client && (!(ent->spawnflags & 0x10) || activator->client->ps.persistant[PERS_SCORE] == ent->count)) {
    if (activator->client->ps.persistant[PERS_SCORE] < ent->count) {
      if (!(ent->spawnflags & 4)) {
        if (ent->count - activator->client->ps.persistant[PERS_SCORE] == 1) {
          text = va("cpsm \"%i more frag needed\n\"",
                    ent->count - activator->client->ps.persistant[PERS_SCORE]);
          trap_SendServerCommand(activator-g_entities, text);
        }
        else {
          text = va("cpsm \"%i more frags needed\n\"",
                    ent->count - activator->client->ps.persistant[PERS_SCORE]);
          trap_SendServerCommand(activator-g_entities, text);
        }
      }
    }
    else {
      if ((ent->s).generic1 != 0) {
        if ((float)level.time < ent->wait) {
          ent->wait = (float)(level.time + 1000);
          return;
        }
        ent->wait = (float)(level.time + 1000);
      }
      if ((ent->spawnflags & 1U) != 0) {
        activator->client->ps.persistant[PERS_SCORE] -= ent->count;
      }
      if ((ent->spawnflags & 8U) != 0) {
        activator->client->ps.persistant[PERS_SCORE] = 0;
      }
      G_UseTargets(ent,activator);
    }
  }
  return;
}