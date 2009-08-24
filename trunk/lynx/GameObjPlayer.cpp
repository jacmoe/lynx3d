#include "GameObjPlayer.h"
#include "GameObjZombie.h"

#ifdef _DEBUG
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK,__FILE__, __LINE__)
#endif

CGameObjPlayer::CGameObjPlayer(CWorld* world) : CGameObj(world)
{
    m_prim_triggered = 0;
    m_prim_triggered_time = 0;
}

CGameObjPlayer::~CGameObjPlayer(void)
{
}

void CGameObjPlayer::CmdFire(bool active)
{
    if(!m_prim_triggered && active)
        OnCmdFire();
    m_prim_triggered = active;
}

void CGameObjPlayer::OnCmdFire()
{
    if(GetWorld()->GetLeveltime() - m_prim_triggered_time < 50)
        return;
    m_prim_triggered_time = GetWorld()->GetLeveltime();

    world_obj_trace_t trace;
    vec3_t dir;
    GetDir(&dir, NULL, NULL); // richtung holen
    trace.dir = -dir;
    trace.start = GetOrigin();
    trace.excludeobj = GetID();
    if(GetWorld()->TraceObj(&trace))
    {
        CGameObj* hitobj = (CGameObj*)GetWorld()->GetObj(trace.objid);
        assert(hitobj);
        if(!hitobj->IsClient())
        {
            hitobj->DealDamage(20, trace.hitpoint, trace.dir);
        }
    }
}