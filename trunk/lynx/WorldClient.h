#pragma once
#include "world.h"

class CWorldClient :
	public CWorld
{
public:
	CWorldClient(void);
	~CWorldClient(void);

	CObj* GetLocalObj();
	void SetLocalObj(int id);

	void Update(const float dt);

private:
	CObj* m_localobj; // Objekt mit dem dieser Client verkn�pft ist
	CObj m_ghostobj; // Geister Objekt, falls der Client mit keinem Objekt verkn�pft ist
};
