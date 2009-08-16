#include <string.h>
#include "lynx.h"
#include "Obj.h"
#include <math.h>

#ifdef _DEBUG
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK,__FILE__, __LINE__)
#endif

#define OBJ_STATE_ORIGIN         (1 <<  0)
#define OBJ_STATE_VEL            (1 <<  1)
#define OBJ_STATE_ROT            (1 <<  2)
#define OBJ_STATE_RADIUS         (1 <<  3)
#define OBJ_STATE_RESOURCE       (1 <<  4)
#define OBJ_STATE_ANIMATION      (1 <<  5)
#define OBJ_STATE_NEXTANIMATION  (1 <<  6)
#define OBJ_STATE_EYEPOS         (1 <<  7)

#define OBJ_STATE_FULLUPDATE     ((1 << 8)-1)

int CObj::m_idpool = 0;

CObj::CObj(CWorld* world)
{
	assert(world);
	m_id = ++m_idpool;
    assert(m_id < USHRT_MAX); // Die id wird im Netzwerk als WORD übertragen
    state.radius = 2*lynxmath::SQRT_2;
	state.eyepos = vec3_t(0,0.5f,0);
    state.animation = 0;
    state.nextanimation = 0;
	m_mesh = NULL;
	m_world = world;
    UpdateMatrix();
	m_stream.Resize(512);

    // Local Attributes
    m_locIsOnGround = false;
    m_opaque = NULL;
}

CObj::~CObj(void)
{
    if(m_opaque)
        delete m_opaque;
}

void CObj::SetOpaque(CObjOpaque* opaque)
{
    if(m_opaque)
        delete m_opaque;
    m_opaque = opaque;
}

void CObj::UpdateMatrix()
{
    state.rot.ToMatrix(m);
}

float CObj::GetRadius()
{
	return state.radius;
}

void CObj::SetRadius(float radius)
{
    state.radius = radius;
}

std::string CObj::GetResource()
{
	return state.resource;
}

void CObj::SetResource(std::string resource)
{
	assert(resource.size() < USHRT_MAX);
	if(resource.size() >= USHRT_MAX)
		return;

	if(state.resource != resource)
	{
		state.resource = resource;
		UpdateProperties();
		if(m_mesh)
		{
			state.radius = m_mesh->GetSphere();
		}
	}
}

INT16 CObj::GetAnimation()
{
	return state.animation;
}

void CObj::SetAnimation(INT16 animation)
{
	if(animation != state.animation)
	{
		state.animation = animation;
        state.nextanimation = animation;
		UpdateProperties();
	}
}

INT16 CObj::GetNextAnimation()
{
	return state.nextanimation;
}

void CObj::SetNextAnimation(INT16 animation)
{
	if(animation != state.nextanimation)
	{
		state.nextanimation = animation;
        assert(m_mesh);
        if(m_mesh)
            m_mesh->SetNextAnimation(&m_mesh_state, state.nextanimation);
	}
}

vec3_t CObj::GetEyePos()
{
	return state.eyepos;
}

void CObj::SetEyePos(const vec3_t& eyepos)
{
    state.eyepos = eyepos;
}

int CObj::GetAnimationFromName(const char* name) const
{
    assert(m_mesh);
    if(m_mesh)
        return m_mesh->FindAnimation(name);
    else
        return -1;
}

// DELTA COMPRESSION CODE (ugly) ------------------------------------

int DeltaDiffVec3(const vec3_t* newstate,
                  const vec3_t* oldstate,
                  const DWORD flagparam, 
                  DWORD* updateflags,
                  CStream* stream)
{
    if(!oldstate || (*newstate != *oldstate)) {
        *updateflags |= flagparam;
        if(stream) stream->WriteVec3(*newstate);
        return STREAM_SIZE_VEC3;
    }
    return 0;
}

int DeltaDiffQuat(const quaternion_t* newstate,
                  const quaternion_t* oldstate,
                  const DWORD flagparam, 
                  DWORD* updateflags,
                  CStream* stream)
{
    if(!oldstate || (*newstate != *oldstate)) {
        *updateflags |= flagparam;
        if(stream) stream->WriteQuat(*newstate);
        return sizeof(quaternion_t);
    }
    return 0;
}

int DeltaDiffFloat(const float* newstate,
                   const float* oldstate,
                   const DWORD flagparam, 
                   DWORD* updateflags,
                   CStream* stream)
{
    if(!oldstate || !(*newstate == *oldstate)) {
        *updateflags |= flagparam;
        if(stream) stream->WriteFloat(*newstate);
        return sizeof(float);
    }
    return 0;
}

int DeltaDiffInt16(const INT16* newstate,
                   const INT16* oldstate,
                   const DWORD flagparam, 
                   DWORD* updateflags,
                   CStream* stream)
{
    if(!oldstate || *newstate != *oldstate) {
        *updateflags |= flagparam;
        if(stream) stream->WriteInt16(*newstate);
        return sizeof(INT16);
    }
    return 0;
}

int DeltaDiffString(const std::string* newstate,
                    const std::string* oldstate,
                    const DWORD flagparam, 
                    DWORD* updateflags,
                    CStream* stream)
{
    if(!oldstate || *newstate != *oldstate) {
        *updateflags |= flagparam;
        if(stream) stream->WriteString(*newstate);
        return (int)CStream::StringSize(*newstate);
    }
    return 0;
}

int DeltaDiffDWORD(const DWORD* newstate,
                   const DWORD* oldstate,
                   const DWORD flagparam, 
                   DWORD* updateflags,
                   CStream* stream)
{
    if(!oldstate || *newstate != *oldstate) {
        *updateflags |= flagparam;
        if(stream) stream->WriteDWORD(*newstate);
        return sizeof(DWORD);
    }
    return 0;
}

bool CObj::Serialize(bool write, CStream* stream, int id, const obj_state_t* oldstate)
{
    assert(!(!write && oldstate));
    assert(stream);
    DWORD updateflags = 0;

	if(write)
	{
        assert(GetID() == id);
		assert(id < USHRT_MAX);
		m_stream.ResetWritePosition();

		DeltaDiffVec3(&state.origin,            oldstate ? &oldstate->origin : NULL,        OBJ_STATE_ORIGIN,       &updateflags, &m_stream);
        DeltaDiffVec3(&state.vel,               oldstate ? &oldstate->vel : NULL,           OBJ_STATE_VEL,          &updateflags, &m_stream);
        DeltaDiffQuat(&state.rot,               oldstate ? &oldstate->rot : NULL,           OBJ_STATE_ROT,          &updateflags, &m_stream);
        DeltaDiffFloat(&state.radius,           oldstate ? &oldstate->radius : NULL,        OBJ_STATE_RADIUS,       &updateflags, &m_stream);
        DeltaDiffString(&state.resource,        oldstate ? &oldstate->resource : NULL,      OBJ_STATE_RESOURCE,     &updateflags, &m_stream);
        DeltaDiffInt16(&state.animation,        oldstate ? &oldstate->animation : NULL,     OBJ_STATE_ANIMATION,    &updateflags, &m_stream);
        DeltaDiffInt16(&state.nextanimation,    oldstate ? &oldstate->nextanimation : NULL, OBJ_STATE_NEXTANIMATION,&updateflags, &m_stream);
        DeltaDiffVec3(&state.eyepos,            oldstate ? &oldstate->eyepos : NULL,        OBJ_STATE_EYEPOS,       &updateflags, &m_stream);
        stream->WriteDWORD(updateflags);
        stream->WriteStream(m_stream);

        assert(oldstate ? 1 : (updateflags == OBJ_STATE_FULLUPDATE));
	}
	else
	{
        stream->ReadDWORD(&updateflags);

        if(updateflags & OBJ_STATE_ORIGIN)
            stream->ReadVec3(&state.origin);
        if(updateflags & OBJ_STATE_VEL)
            stream->ReadVec3(&state.vel);
        if(updateflags & OBJ_STATE_ROT)
        {
            stream->ReadQuat(&state.rot);
            UpdateMatrix();
        }
        if(updateflags & OBJ_STATE_RADIUS)
            stream->ReadFloat(&state.radius);
        if(updateflags & OBJ_STATE_RESOURCE)
            stream->ReadString(&state.resource);
        if(updateflags & OBJ_STATE_ANIMATION)
            stream->ReadInt16(&state.animation);
        if(updateflags & OBJ_STATE_NEXTANIMATION)
            stream->ReadInt16(&state.nextanimation);
        if(updateflags & OBJ_STATE_EYEPOS)
            stream->ReadVec3(&state.eyepos);

        m_id = id;
        if(updateflags & OBJ_STATE_RESOURCE || updateflags & OBJ_STATE_ANIMATION ||
            updateflags & OBJ_STATE_NEXTANIMATION)
		{
			UpdateProperties();
		}
	}

	return updateflags != 0;
}

void CObj::SetObjState(const obj_state_t* objstate, int id)
{
	m_id = id;
    bool resourcechange = objstate->resource != state.resource || 
                          objstate->animation != state.animation ||
                          objstate->nextanimation != state.nextanimation;
	state = *objstate;
    if(resourcechange)
	    UpdateProperties();
}

void CObj::CopyObjStateFrom(const CObj* source)
{
    SetObjState(&source->state, m_id);
}

void CObj::UpdateProperties()
{
	m_mesh = m_world->GetResourceManager()->GetModel(state.resource);
	if(state.animation >= 0)
        m_mesh->SetAnimation(&m_mesh_state, state.animation);
	else
		m_mesh->SetAnimation(&m_mesh_state, 0);
    m_mesh->SetNextAnimation(&m_mesh_state, state.nextanimation);
	UpdateMatrix();
}