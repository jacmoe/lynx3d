#pragma once

class CWorld;
#include <map>
#include <list>
#include "Obj.h"
#include "BSPTree.h"
#include "ResourceManager.h"

/*
    CWorld ist der wichtigste Kern in der Lynx Engine.
    Der Zustand einer Welt wird Datenm��ig in der 
    world_state_t Struktur festgehalten und von
    der CWorld Klasse verwaltet.

    Update() aktualisiert die Welt bei jedem Frame
    Mit Serialize() kann die Welt als Datenstrom abgespeichert werden bzw. �ber das Netzwerk verschickt werden

 */

#define OBJMAPTYPE	std::map<int, CObj*>
#define OBJITER		std::map<int, CObj*>::iterator

/*
    if you change world_state_t, change:
    - GenerateWorldState
    - Serialize
 */
struct world_state_t
{
    DWORD   leveltime; // in ms
    DWORD   worldid; // fortlaufende nummer
    std::string level; // Pfad zu Level

	std::vector<obj_state_t> objstates;
    std::map<int,int> objindex; // ID zu objstates index tabelle
};

struct world_obj_trace_t
{
    // Input
	vec3_t	start; // start point
    vec3_t  dir; // end point = start + dir
    int     excludeobj; // welche objekt wird vom strahl ignoriert

    // Output
	float	f; // impact = start + f*dir
	vec3_t  hitpoint;
    vec3_t  hitnormal;
    int     objid;
};

class CWorld
{
public:
	CWorld(void);
	virtual ~CWorld(void);

    virtual bool IsClient() const { return false; } // Hilfsfunktion, um bestimmte Aktionen f�r einen Server nicht auszuf�hren (Texturen laden)

	virtual void Update(const float dt, const DWORD ticks); // Neues Frame berechnen

	void	AddObj(CObj* obj, bool inthisframe=false); // Objekt in Welt hinzuf�gen. Speicher wird automatisch von World freigegeben
	void	DelObj(int objid); // Objekt aus Welt entfernen. Wird beim n�chsten Frame gel�scht

	CObj*	GetObj(int objid); // Objekt mit dieser ID suchen

	int		GetObjCount() const { return (int)m_objlist.size(); } // Anzahl der Objekte in Welt
	OBJITER ObjBegin() { return m_objlist.begin(); } // Begin Iterator
	OBJITER ObjEnd() { return m_objlist.end(); } // End Iterator

	virtual bool Serialize(bool write, CStream* stream, const world_state_t* oldstate=NULL); // Komplette Welt in einen Byte-Stream schreiben. true, wenn sich welt gegen�ber oldstate ver�ndert hat

    bool    LoadLevel(const std::string path); // Level laden und BSP Tree vorbereiten
    const virtual CBSPTree* GetBSP() const { return &m_bsptree; }
    DWORD   GetLeveltime() const { return state.leveltime; } // Levelzeit, beginnt bei 0
    DWORD   GetWorldID() const { return state.worldid; } // WorldID erh�ht sich bei jedem Update() aufruf um 1
    world_state_t GetWorldState(); // 

	virtual CResourceManager* GetResourceManager() { return &m_resman; }

	void	ObjMove(CObj* obj, const float dt) const; // Objekt bewegen + Kollisionserkennung

    bool    TraceObj(world_obj_trace_t* trace);


protected:
	
	CResourceManager m_resman;
	world_state_t state;
    DWORD    m_leveltimestart;
    CBSPTree m_bsptree;

	OBJMAPTYPE m_objlist;
	void	UpdatePendingObjs(); // Entfernt zu l�schende Objekte und f�gt neue hinzu (siehe m_addobj und removeobj)
	void	DeleteAllObjs(); // L�scht alle Objekte aus m_objlist und gibt auch den Speicher frei

	std::list<CObj*> m_addobj; // Liste von Objekten die im n�chsten Frame hinzugef�gt werden
	std::list<int> m_removeobj; // Liste von Objekten die im n�chsten Frame gel�scht werden
};
