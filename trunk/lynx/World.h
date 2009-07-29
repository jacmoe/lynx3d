#pragma once

class CWorld;
#include <map>
#include <list>
#include "Obj.h"
#include "ResourceManager.h"
#include "BSPTree.h"

#define OBJMAPTYPE	std::map<int, CObj*>
#define OBJITER		std::map<int, CObj*>::iterator

class CWorld
{
public:
	CWorld(void);
	virtual ~CWorld(void);

	virtual void Update(const float dt); // Neues Frame berechnen

	void	AddObj(CObj* obj, bool inthisframe=false); // Objekt in Welt hinzuf�gen. Speicher wird automatisch von World freigegeben
	void	DelObj(int objid); // Objekt aus Welt entfernen. Wird beim n�chsten Frame gel�scht
	
	CObj*	GetObj(int objid); // Objekt mit dieser ID suchen

	int		GetObjCount() { return (int)m_objlist.size(); } // Anzahl der Objekte in Welt
	OBJITER ObjBegin() { return m_objlist.begin(); } // Begin Iterator
	OBJITER ObjEnd() { return m_objlist.end(); } // End Iterator

	int		Serialize(bool write, CStream* stream); // Komplette Welt in einen Byte-Stream schreiben
	int		SerializePositions(bool write, CStream* stream, int objpvs, int mtu, int* lastobj); // Nur Positionsupdates schreiben. objpvs = nur objekte ber�cksichtigen, die sichtbar f�r obj sind
	/*
		SerializePositions

		write: true
		Schreibt alle Positionen von Objekten die f�r das Objekt (objpvs) potentiell 
		sichtbar sind in einen Bytestream.
		mtu gibt an, wie viele Bytes maximal in den Stream geschrieben werden
		d�rfen.
		Wenn nicht alle Objekte in den Stream geschrieben werden k�nnen,
		wird in lastobj die ID vom n�chsten Objekt geschrieben. Bei erneutem Aufruf
		von SerializePositions wird an dieser Stelle weitergemacht. Wenn lastobj auf 
		ein int mit dem Wert 0 zeigt, wird wieder von vorne angefangen.
		return int: anzahl geschriebener objekte
		nach dem letzten objekt wird lastobj auf den wert -1 gesetzt

		write: false
		Alle Objektpositionen aus diesem Stream werden aktualisiert.
		return int: anzahl gelesener objekte
		return -1: fehler beim lesen (stream ung�ltig)
	*/

	CResourceManager m_resman;
	CBSPTree m_bsptree;

protected:

	OBJMAPTYPE m_objlist;
	void	UpdatePendingObjs(); // Entfernt zu l�schende Objekte und f�gt neue hinzu (siehe m_addobj und removeobj)
	void	DeleteAllObjs(); // L�scht alle Objekte aus m_objlist und gibt auch den Speicher frei

	void	ObjCollision(CObj* obj, float dt);

	std::list<CObj*> m_addobj; // Liste von Objekten die im n�chsten Frame hinzugef�gt werden
	std::list<int> m_removeobj; // Liste von Objekten die im n�chsten Frame gel�scht werden
};
