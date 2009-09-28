#include <assert.h>
#include "math/mathconst.h"
#include "World.h"
#include <math.h>
#include <list>
#include "lynxsys.h"

#ifdef _DEBUG
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK,__FILE__, __LINE__)
#endif

#pragma warning(disable:4355)
CWorld::CWorld(void) : m_resman(this)
{
    state.worldid = 0;
    m_leveltimestart = CLynxSys::GetTicks();
    state.leveltime = 0;
}

CWorld::~CWorld(void)
{
	DeleteAllObjs();
}

void CWorld::AddObj(CObj* obj, bool inthisframe)
{
	assert(obj && !GetObj(obj->GetID()));
	assert(GetObjCount() < USHRT_MAX);

	if(inthisframe)
		m_objlist[obj->GetID()] = obj;
	else
		m_addobj.push_back(obj);
}

void CWorld::DelObj(int objid)
{
	assert(GetObj(objid));
	if(GetObj(objid))
		m_removeobj.push_back(objid);
}

CObj* CWorld::GetObj(int objid)
{
	OBJITER iter;
	iter = m_objlist.find(objid);
	if(iter == m_objlist.end())
		return NULL;
	return (*iter).second;
}

void CWorld::DeleteAllObjs()
{
	OBJITER iter;

	for(iter = ObjBegin();iter!=ObjEnd();iter++)
		delete (*iter).second;
	m_objlist.clear();
}

const std::vector<CObj*> CWorld::GetNearObj(const vec3_t& origin, const float radius, const int exclude, const int type) const
{
    std::vector<CObj*> objlist;
    const float radius2 = radius * radius;
    CObj* obj;
	OBJITERCONST iter;
	for(iter = m_objlist.begin();iter!=m_objlist.end();iter++)
    {
        obj = (*iter).second;
        if(obj->GetID() != exclude && (obj->GetOrigin() - origin).AbsSquared() < radius2 &&
            ((obj->GetType() == type) || type < 0 ))
            objlist.push_back(obj);
    }
    return objlist;
}


void CWorld::Update(const float dt, const DWORD ticks)
{
    if(!IsClient())
    {
        state.leveltime = ticks - m_leveltimestart;
        state.worldid++;
    }

	UpdatePendingObjs();

    if(!m_bsptree.IsLoaded())
		return;
}

#define GRAVITY             (100.00f) // sollte das als Welt Eigenschaft aufgenommen werden?
const static vec3_t gravity(0, -GRAVITY, 0);
#define STOP_EPSILON		(0.01f)
void CWorld::ObjMove(CObj* obj, const float dt) const
{
    bsp_sphere_trace_t trace;
    vec3_t p1 = obj->GetOrigin(); // Startpunkt
    vec3_t p2 = p1; // Gew�nschter Endpunkt
    vec3_t vel = obj->GetVel();
    vec3_t q; // Schnittpunkt mit Levelgeometrie
    vec3_t p3; // Endpunkt nach "slide"

    if(fabsf(vel.y) > 999.9f)
    {
        fprintf(stderr, "World error: obj in free fall\n");
        vel.y = 0;
        assert(0);
        return;
    }

    if(!(obj->GetFlags() & OBJ_FLAGS_NOGRAVITY)) // Objekt reagiert auf Gravity
    {
        vel += gravity*dt;
        p2  += 0.5f*dt*dt*gravity;
    }
    
    p2 += vel*dt;
    trace.radius = obj->GetRadius();
    bool groundhit = false;

    for(int i=0;(p2 - p1) != vec3_t::origin && i < 10; i++)
    {
        //assert(i < 9); // sollte nicht passieren

        trace.start = p1;
        trace.dir = p2 - p1;
        trace.f = MAX_TRACE_DIST;
        GetBSP()->TraceSphere(&trace);
        if(trace.f < 1.0f)
        {
            q = trace.start + trace.f*trace.dir + 
                trace.p.m_n * STOP_EPSILON;
            if(obj->GetFlags() & OBJ_FLAGS_ELASTIC) // bounce
            {
                vel = 0.6f*(vel - 2.0f*(vel*trace.p.m_n)*trace.p.m_n);
                p3 = q;
            }
            else // slide
            {
                p3 = p2 -((p2 - q)*trace.p.m_n)*trace.p.m_n;

                if(trace.p.m_n * vec3_t::yAxis > lynxmath::SQRT_2_HALF) // unter 45� neigung bleiben wir stehen
                    groundhit = true;
            }

            if(fabsf(q.y-p1.y)<STOP_EPSILON*100*dt)
                q.y = p1.y;

            if((p3-q).Abs()<STOP_EPSILON*1000*dt)
            {
                p2 = q;
                break;
            }

            p1 = q;
            p2 = p3;
        }
        else
            break;
    }
 
    if(groundhit)
    {
        vel.y = 0;
    }
    obj->m_locIsOnGround = groundhit;

    obj->SetOrigin(p2);
    obj->SetVel(vel);
}

bool CWorld::TraceObj(world_obj_trace_t* trace)
{
    OBJITER iter;
    float minf = MAX_TRACE_DIST;
    float cf;
    CObj* obj;
    CObj* ignore = GetObj(trace->excludeobj);
    const float radius = ignore->GetRadius();
    vec3_t origin;
    int objhit = -1;

    for(iter = ObjBegin(); iter != ObjEnd(); iter++)
    {
        obj = (*iter).second;
        if(obj->GetID() == ignore->GetID() || obj->GetRadius() < lynxmath::EPSILON)
            continue;
        origin = obj->GetOrigin();
        if(!vec3_t::RaySphereIntersect(trace->start, trace->dir,
                                   origin, obj->GetRadius(),
                                   &cf))
        {
            continue;
        }
        
        if(cf >= -radius && cf < minf)
        {
            objhit = obj->GetID();
            minf = cf;
        }
    }

    if(objhit == -1)
    {
        trace->objid = -1;
        trace->f = MAX_TRACE_DIST;
        return false;
    }

    bsp_sphere_trace_t spheretrace;
    spheretrace.start = trace->start;
    spheretrace.dir = trace->dir * minf;
    spheretrace.radius = 0.01f;
    GetBSP()->TraceSphere(&spheretrace);
    if(spheretrace.f <= 1.0f)
    {
        trace->f = spheretrace.f;
        trace->hitpoint = spheretrace.start + spheretrace.f*spheretrace.dir;
        trace->hitnormal = spheretrace.p.m_n;
        trace->objid = -1;

        return false;
    }
    else
    {
        obj = GetObj(objhit);
        trace->f = minf;
        trace->hitpoint = trace->start + minf*trace->dir;
        trace->hitnormal = (trace->hitpoint - obj->GetOrigin()).Normalized();
        trace->objid = objhit;

        return true;
    }
}

void CWorld::UpdatePendingObjs()
{
	if(m_removeobj.size() > 0)
	{
		// Zu l�schende Objekte entfernen
		std::list<int>::iterator remiter;
		OBJITER iter;
		for(remiter=m_removeobj.begin();remiter!=m_removeobj.end();remiter++)
		{
			iter = m_objlist.find((*remiter));
			assert(iter != m_objlist.end());
			delete (*iter).second;
			m_objlist.erase(iter);
		}
		m_removeobj.clear();
	}

	if(m_addobj.size() > 0)
	{
		// Objekte f�r das Frame hinzuf�gen
		std::list<CObj*>::iterator additer;
		for(additer=m_addobj.begin();additer!=m_addobj.end();additer++)
		{
			m_objlist[(*additer)->GetID()] = (*additer);
		}
		m_addobj.clear();
	}
}

bool CWorld::LoadLevel(const std::string path)
{
    bool success = m_bsptree.Load(path, IsClient() ? GetResourceManager() : NULL);
    if(success)
        state.level = m_bsptree.GetFilename();
    return success;
}

// DELTA COMPRESSION CODE (ugly atm) ------------------------------------

#define WORLD_STATE_WORLDID			(1 <<  0)
#define WORLD_STATE_LEVELTIME		(1 <<  1)
#define WORLD_STATE_LEVEL			(1 <<  2)

#define WORLD_STATE_NO_REAL_CHANGE	(WORLD_STATE_WORLDID|WORLD_STATE_LEVELTIME) // worldid und leveltime �ndern sich sowieso immer
#define WORLD_STATE_FULLUPDATE		((1 <<  3)-1)

bool CWorld::Serialize(bool write, CStream* stream, const world_state_t* oldstate)
{
    assert(stream);
	int size = 0;
	CObj* obj;
    OBJITER iter;
	int changes = 0;

	if(write)
	{
        DWORD updateflags = 0;
        CStream tempstream = stream->GetStream();
        stream->WriteAdvance(sizeof(DWORD)); // An dieser Stelle sollten als DWORD die Updateflags stehen, diese kennen wir erst, nachdem wir sie geschrieben haben

        // Zuerst in tempstream schreiben um gleichzeitig die updateflags zu erkennen
        DeltaDiffDWORD(&state.worldid, oldstate ? &oldstate->worldid : NULL, WORLD_STATE_WORLDID, &updateflags, stream);
        DeltaDiffDWORD(&state.leveltime, oldstate ? &oldstate->leveltime : NULL, WORLD_STATE_LEVELTIME, &updateflags, stream);
        DeltaDiffString(&state.level, oldstate ? &oldstate->level : NULL, WORLD_STATE_LEVEL, &updateflags, stream);
		// [NEUE ATTRIBUTE HIER]

		if(updateflags > WORLD_STATE_NO_REAL_CHANGE)
			changes++;
		
        assert(oldstate ? 1 : (updateflags == WORLD_STATE_FULLUPDATE)); // muss zwingend eingehalten werden
        tempstream.WriteDWORD(updateflags); // Jetzt kennen wir die Updateflags und k�nnen sie in den tats�chlichen stream schreiben

        // Alle Objekte schreiben
        assert(GetObjCount() < USHRT_MAX);
        stream->WriteWORD((WORD)GetObjCount());

        obj_state_t obj_oldstate;
        obj_state_t* p_obj_oldstate;
		for(iter = m_objlist.begin();iter!=m_objlist.end();iter++)
		{
			obj = (*iter).second;
            p_obj_oldstate = NULL;
            if(oldstate) // Delta Compression
            {
                if(oldstate->ObjStateExists(obj->GetID()))
                {
                    oldstate->GetObjState(obj->GetID(), obj_oldstate);
                    p_obj_oldstate = &obj_oldstate;
                }
            }
            stream->WriteWORD(obj->GetID());
            if(obj->Serialize(true, stream, obj->GetID(), p_obj_oldstate))
				changes++;
		}
	}
	else
	{
        DWORD updateflags;
        DWORD worldid;
        std::string level;
        WORD objcount;
        WORD objid;

        stream->ReadDWORD(&updateflags);
        assert(updateflags > 0);
        assert(updateflags & WORLD_STATE_WORLDID);
        if(updateflags & WORLD_STATE_WORLDID)
            stream->ReadDWORD(&worldid);
		else
			return false;
        if(worldid < state.worldid)
        {
            assert(0); // anschauen, ob ok
            return false;
        }
        state.worldid = worldid;
        if(updateflags & WORLD_STATE_LEVELTIME)
            stream->ReadDWORD(&state.leveltime);
        if(updateflags & WORLD_STATE_LEVEL)
        {
            stream->ReadString(&level);
            assert(level.size() > 0);
            if(level != m_bsptree.GetFilename())
            {
                if(LoadLevel(level)==false)
			    {
    				// FIXME error handling
					assert(0);
				    return false;
			    }
            }
        }
		// [NEUE ATTRIBUTE HIER]
		if(updateflags > WORLD_STATE_NO_REAL_CHANGE)
			changes++;

        stream->ReadWORD(&objcount);
        assert(objcount < USHRT_MAX);
        std::map<int,int> objread; // objekte die gelesen wurden, was hier nicht steht, muss gel�scht werden FIXME: hash_map verwenden?
        for(int i=0;i<objcount;i++)
        {
            stream->ReadWORD(&objid);
            obj = GetObj(objid);
            if(!obj)
            {
                obj = new CObj(this);
				changes += obj->Serialize(false, stream, objid) ? 1 : 0;
                AddObj(obj);
            }
            else
				changes += obj->Serialize(false, stream, objid) ? 1 : 0;
            assert(objread.find(objid) == objread.end()); // darf nicht schon im stream sein
            objread[obj->GetID()] = obj->GetID();
        }
        // Alle Objekte l�schen, die in dem neuen State nicht mehr vorhanden sind
        for(iter=m_objlist.begin();iter!=m_objlist.end();iter++)
        {
            obj = (*iter).second;
            if(objread.find(obj->GetID()) != objread.end())
                continue;
            //assert(0); // nur mal zum sehen, ob hier alles klappt. danach zeile l�schen
            DelObj(obj->GetID());
        }

		UpdatePendingObjs();
	}

	return changes > 0;
}

world_state_t CWorld::GetWorldState()
{
	world_state_t worldstate = state;

    assert(worldstate.GetObjCount() == 0);

    OBJITER iter;
	for(iter = m_objlist.begin();iter!=m_objlist.end();iter++)
	{
		CObj* obj = (*iter).second;
        worldstate.AddObjState(obj->GetObjState(), obj->GetID());
    }

	return worldstate;
}

// world_state_t Struktur Methoden (f�r sicheren Zugriff auf Objekt Vektor in world_state_t)

void world_state_t::AddObjState(obj_state_t objstate, const int id)
{
    assert(!ObjStateExists(id));
    objstates.push_back(objstate);
    objindex[id] = (int)objstates.size()-1;    
}

bool world_state_t::ObjStateExists(const int id) const
{
    WORLD_STATE_CONSTOBJITER indexiter = objindex.find(id);
    return indexiter != objindex.end();
}

bool world_state_t::GetObjState(const int id, obj_state_t& objstate) const
{
    WORLD_STATE_CONSTOBJITER indexiter = objindex.find(id);
    if(indexiter == objindex.end())
        return false;
    objstate = objstates[indexiter->second];
    return true;
}
