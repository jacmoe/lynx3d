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
#include "Sound.h"
#include <memory>
#include "ParticleSystem.h"

#define OBJ_FLAGS_ELASTIC       (1 << 0)
#define OBJ_FLAGS_NOGRAVITY     (1 << 1)

#define OBJFLAGTYPE             uint8_t


// Serialize Helper Functions: Compare if newstate != oldstate, update updateflags with flagparam and write to stream (if not null)
// FIXME: use template function
int DeltaDiffVec3(const vec3_t* newstate,
                  const vec3_t* oldstate,
                  const uint32_t flagparam, 
                  uint32_t* updateflags,
                  CStream* stream);
int DeltaDiffQuat(const quaternion_t* newstate,
                  const quaternion_t* oldstate,
                  const uint32_t flagparam, 
                  uint32_t* updateflags,
                  CStream* stream);
int DeltaDiffFloat(const float* newstate,
                   const float* oldstate,
                   const uint32_t flagparam, 
                   uint32_t* updateflags,
                   CStream* stream);
int DeltaDiffInt16(const int16_t* newstate,
                   const int16_t* oldstate,
                   const uint32_t flagparam, 
                   uint32_t* updateflags,
                   CStream* stream);
int DeltaDiffString(const std::string* newstate,
                    const std::string* oldstate,
                    const uint32_t flagparam, 
                    uint32_t* updateflags,
                    CStream* stream);
int DeltaDiffDWORD(const uint32_t* newstate,
                   const uint32_t* oldstate,
                   const uint32_t flagparam, 
                   uint32_t* updateflags,
                   CStream* stream);
int DeltaDiffBytes(const uint8_t* newstate,
                   const uint8_t* oldstate,
                   const uint32_t flagparam, 
                   uint32_t* updateflags,
                   CStream* stream,
                   const int size);


// If you change this, update the Serialize function
struct obj_state_t
{
    vec3_t          origin;         // Position
    vec3_t          vel;            // Direction/Velocity
    quaternion_t    rot;            // Rotation

    float           radius;
    std::string     resource;
    int16_t         animation;
    int16_t         nextanimation;
    vec3_t          eyepos;
    OBJFLAGTYPE     flags;
    std::string     particles;      // Partikelsystem, das an dieses Objekt gebunden ist. Config String in der Form: "blood|dx=0.1,dy=0.6,dz=23".
};

class CObj
{
public:
    CObj(CWorld* world);
    virtual ~CObj(void);

    int         GetID() const { return m_id; }
    virtual int GetType() { return 0; } // poor man's rtti

    bool        Serialize(bool write, CStream* stream, int id, const obj_state_t* oldstate=NULL); // Objekt in einen Byte-Stream schreiben. Wenn oldstate ungleich NULL, wird nur die Differenz geschrieben, gibt true zur�ck, wenn sich objekt durch ge�ndert hat (beim lesen) oder wenn es sich von oldstate unterscheidet

    obj_state_t GetObjState() const { return state; }
    void        SetObjState(const obj_state_t* objstate, int id);
    void        CopyObjStateFrom(const CObj* source); // Eigenschaften von anderem Objekt kopieren

    const vec3_t GetOrigin() const { return state.origin; }
    void         SetOrigin(const vec3_t& origin) { state.origin = origin; }
    const vec3_t GetVel() const { return state.vel; }
    void         SetVel(const vec3_t& velocity) { state.vel = velocity; }
    const quaternion_t GetRot() const { return state.rot; }
    void        SetRot(const quaternion_t& rotation);
    void        GetDir(vec3_t* dir, vec3_t* up, vec3_t* side) const;

    float       GetRadius() const; // Max. Object sphere size
    void        SetRadius(float radius);
    std::string GetResource() const;
    void        SetResource(std::string resource);
    int16_t     GetAnimation() const;
    void        SetAnimation(int16_t animation);
    int16_t     GetNextAnimation() const;
    void        SetNextAnimation(int16_t animation);
    vec3_t      GetEyePos() const;
    void        SetEyePos(const vec3_t& eyepos);
    OBJFLAGTYPE GetFlags() const;
    void        SetFlags(OBJFLAGTYPE flags);
    void        AddFlags(OBJFLAGTYPE flags);
    void        RemoveFlags(OBJFLAGTYPE flags);
    void        SetParticleSystem(const std::string psystem);
    std::string GetParticleSystemName() const;

    // Local Attributes
    bool        locGetIsOnGround() const { return m_locIsOnGround; } // Ber�ht das Objekt den Boden? Wird von World::ObjMove gesetzt

    // Rotation
    const matrix_t* GetRotMatrix() const { return &m; } // Direkter Zugriff auf die Rotationsmatrix

    // Model Data
    const CModelMD2* GetMesh() const { return m_mesh; }
    md2_state_t*     GetMeshState() { return &m_mesh_state; }
    int              GetAnimationFromName(const char* name) const;

    // Sound Data
    const CSound*   GetSound() const { return m_sound; }
    sound_state_t*  GetSoundState() { return &m_sound_state; }

    // Particle Systems
    CParticleSystem* GetParticleSystem() { return m_particlesys.get(); }

protected:
    obj_state_t state; // obj_state

    // Animation extension
    CModelMD2*  m_mesh;
    md2_state_t m_mesh_state;
    void        UpdateAnimation();

    // Sound
    CSound*     m_sound;
    sound_state_t m_sound_state;

    // Rotation extension
    void        UpdateMatrix();
    matrix_t    m;

    // Particle extension
    void        UpdateParticles();
    std::auto_ptr<CParticleSystem> m_particlesys;

    // Local Attributes
    bool        m_locIsOnGround;
    friend class CWorld;

    CWorld*     GetWorld() { return m_world; }

private:
    // Don't touch these
    int         m_id;
    CWorld*     m_world;

    static int m_idpool;
};
