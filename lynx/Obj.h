#pragma once

class CObj;
struct obj_state_t;
#include "math/vec3.h"
#include "math/matrix.h"
#include "math/quaternion.h"
#include "Stream.h"
#include <string>
#include "World.h"
#include "ModelMD2.h"

class CObjOpaque // User-Daten von CObj (werden nicht �ber das Netzwerk �bertragen)
{
public:
    CObjOpaque() {}
    virtual ~CObjOpaque() {}
};

// Serialize Helper Functions: Compare if newstate != oldstate, update updateflags with flagparam and write to stream (if not null)
int DeltaDiffVec3(const vec3_t* newstate,
                  const vec3_t* oldstate,
                  const DWORD flagparam, 
                  DWORD* updateflags,
                  CStream* stream);
int DeltaDiffQuat(const quaternion_t* newstate,
                  const quaternion_t* oldstate,
                  const DWORD flagparam, 
                  DWORD* updateflags,
                  CStream* stream);
int DeltaDiffFloat(const float* newstate,
                   const float* oldstate,
                   const DWORD flagparam, 
                   DWORD* updateflags,
                   CStream* stream);
int DeltaDiffInt16(const INT16* newstate,
                   const INT16* oldstate,
                   const DWORD flagparam, 
                   DWORD* updateflags,
                   CStream* stream);
int DeltaDiffString(const std::string* newstate,
                    const std::string* oldstate,
                    const DWORD flagparam, 
                    DWORD* updateflags,
                    CStream* stream);
int DeltaDiffDWORD(const DWORD* newstate,
                   const DWORD* oldstate,
                   const DWORD flagparam, 
                   DWORD* updateflags,
                   CStream* stream);

// If you change this, update the Serialize function
struct obj_state_t
{
	vec3_t	        origin;			// Position
	vec3_t	        vel;     		// Direction/Velocity
	quaternion_t    rot;			// Rotation (x = pitch, y = yaw, z = roll)

	float		    radius;
	std::string     resource;
    INT16           animation;
    INT16           nextanimation;
    vec3_t		    eyepos;
};


class CObj
{
public:
	CObj(CWorld* world);
	virtual ~CObj(void);

	void	    UpdateMatrix();
	matrix_t    m;

	int			GetID() { return m_id; }

	bool		Serialize(bool write, CStream* stream, int id, const obj_state_t* oldstate=NULL); // Objekt in einen Byte-Stream schreiben. Wenn oldstate ungleich NULL, wird nur die Differenz geschrieben, gibt true zur�ck, wenn sich objekt durch ge�ndert hat (beim lesen) oder wenn es sich von oldstate unterscheidet

    obj_state_t GetObjState() const { return state; }
	void		SetObjState(const obj_state_t* objstate, int id);
    void        CopyObjStateFrom(const CObj* source);

    const vec3_t GetOrigin() const { return state.origin; }
    void         SetOrigin(const vec3_t& origin) { state.origin = origin; }
    const vec3_t GetVel() const { return state.vel; }
    void         SetVel(const vec3_t& velocity) { state.vel = velocity; }
    const quaternion_t GetRot() const { return state.rot; }
    void        SetRot(const quaternion_t& rotation) { state.rot = rotation; }

	float		GetRadius(); // Max. Object sphere size
    void        SetRadius(float radius);
	std::string GetResource();
	void		SetResource(std::string resource);
	INT16       GetAnimation();
	void		SetAnimation(INT16 animation);
	INT16       GetNextAnimation();
	void		SetNextAnimation(INT16 animation);
	vec3_t		GetEyePos();
	void		SetEyePos(const vec3_t& eyepos);

    // Local Attributes
    bool        locGetIsOnGround() const { return m_locIsOnGround; }

    int         GetAnimationFromName(const char* name) const;

    // Opaque Data
    CObjOpaque* GetOpaque() { return m_opaque; }
    void        SetOpaque(CObjOpaque* opaque);

    // Model Data
    const CModelMD2* GetMesh() const { return m_mesh; }
    md2_state_t*     GetMeshState() { return &m_mesh_state; }

protected:
	CModelMD2*	m_mesh;
    md2_state_t m_mesh_state;
    
	void		UpdateProperties();
    obj_state_t state;
	CStream		m_stream;

    // Local Attributes
    bool        m_locIsOnGround;
    friend class CWorld;
    CObjOpaque* m_opaque;

private:
    // Don't touch these
	int			m_id;
	CWorld*		m_world;

	static int m_idpool;
};