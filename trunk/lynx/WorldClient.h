#pragma once
#include "world.h"

/*
    CWorldClient ist daf�r zust�ndig, den neuesten Zustand
    der Welt, die vom Server aus gesendet wird zu sammeln.
    D.h. CWorldClient stellt den aktuellsten Zustand der
    Server-Welt dar. Der Zustand setzt sich aus den gesammelten
    Delta-Msgs vom Server zusammen.
    Allerdings ist dieser aktuelle Zustand
    ung�nstig zum Rendern, da falls die Netzwerk-Verz�gerungen 
    schwanken es zu einer unregelm��igen Darstellung kommt.

    Sobald ein Welt-Zustand (world_client_state_t) 
    komplett ist, wird er im History-Buffer gespeichert.
    Diese Zust�nde im History Buffer werden mit ca. 100 ms
    Verz�gerung interpoliert und k�nnen �ber
    CWorldInterp abgerufen werden. CWorldInterp ist daher
    haupts�chlich f�r den Renderer gedacht, der damit ein
    m�glichst fl�ssiges Spiel-Geschehen darstellen kann.
 */

struct worldclient_state_t
{
    world_state_t state;
    DWORD   localtime; // in ms
};

// Interpolierte Welt f�r Renderer

class CWorldClient;
class CWorldInterp : public CWorld
{
public:
	CWorldInterp() { f = 1.0f; }
	~CWorldInterp() {}

    virtual bool    IsClient() const { return true; }

	const virtual CBSPTree* GetBSP() const { return m_pbsp; }
	virtual CResourceManager* GetResourceManager() { return m_presman; }

	void Update(const float dt, const DWORD ticks); // Lineare Interpolation um Schrittweite dt weiter laufen lassen

protected:
	CBSPTree* m_pbsp;
	CResourceManager* m_presman;
	worldclient_state_t state1;
	worldclient_state_t state2;
	float f; // Current scale factor

	friend CWorldClient;
};

class CWorldClient :
	public CWorld
{
public:
	CWorldClient(void);
	~CWorldClient(void);

    virtual bool    IsClient() const { return true; }

	CObj*           GetLocalObj() const;
	void            SetLocalObj(int id);
    CObj*           GetLocalController() { return &m_ghostobj; }

	void            Update(const float dt, const DWORD ticks);

	virtual bool    Serialize(bool write, CStream* stream, const world_state_t* oldstate=NULL);

	CWorld*         GetInterpWorld() { return &m_interpworld; } // Interpolierte Welt

protected:
    void            AddWorldToHistory(); // Aktuelle Welt in den History Buffer schieben
    void            CreateClientInterp(); // Zwischen zwei Zust�nden aus dem History-Buffer die InterpWorld generieren bzw. updaten

    std::list<worldclient_state_t> m_history; // History buffer for Interpolation
	CWorldInterp m_interpworld; // InterpWorld Objekt

private:
	CObj* m_localobj; // Objekt mit dem dieser Client verkn�pft ist
	CObj m_ghostobj; // Controller Object, das wir direkt steuern
};

