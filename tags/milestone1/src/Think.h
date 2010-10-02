#pragma once

#include "Obj.h"

class CThinkFunc
{
public:
    CThinkFunc(uint32_t time, CWorld* world, CObj* obj)
    {
        thinktime = time;
        m_world = world;
        m_obj = obj;
    }
    uint32_t GetThinktime() { return thinktime; }
    void SetThinktime(uint32_t newtime) { thinktime = newtime; }
    virtual bool DoThink(uint32_t leveltime) = 0; // bei r�ckgabe von true wird diese thinkfunc entfernt

protected:
    CWorld* GetWorld() { return m_world; }
    CObj* GetObj() { return m_obj; }
private:
    uint32_t thinktime;
    CWorld* m_world;
    CObj* m_obj;
};

class CThink
{
public:
    ~CThink();
    void AddFunc(CThinkFunc* func); // neue thinkfunc hinzuf�gen
    void RemoveAll(); // alle thinkfuncs l�schen
    void DoThink(uint32_t leveltime); // alle thinkfuncs ausf�hren
private:
    std::list<CThinkFunc*> m_think;
};

