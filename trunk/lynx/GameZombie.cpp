#include <assert.h>
#include "GameZombie.h"
#include "GameObjZombie.h"

#ifdef _DEBUG
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK,__FILE__, __LINE__)
#endif

CGameZombie::CGameZombie(CWorld* world, CServer* server) : CGameLogic(world, server)
{

}

CGameZombie::~CGameZombie(void)
{
}

void CGameZombie::InitGame()
{
    // 1 Testobjekt erstellen
	CGameObjZombie* zombie = new CGameObjZombie(GetWorld());
    zombie->SetOrigin(vec3_t(0.0, 9.0f, 5.0f));
    GetWorld()->AddObj(zombie);

    // Level laden
    GetWorld()->LoadLevel(CLynx::GetBaseDirLevel() + "testlvl/level1.obj");
}

void CGameZombie::Notify(EventNewClientConnected e)
{
	CGameObjPlayer* player;
	player = new CGameObjPlayer(GetWorld());
	player->SetOrigin(vec3_t(0.0f, 2.0f, 0));
    player->SetResource(CLynx::GetBaseDirModel() + "pknight/tris.md2");
	player->SetAnimation(0);
    player->SetEyePos(vec3_t(0,0.65f,0));
    player->SetClientID(e.client->GetID());

	GetWorld()->AddObj(player, true); // In diesem Frame, weil die Welt umgehend vom CServer serialized wird
	e.client->m_obj = player->GetID(); // Mit Client verkn�pfen
}

void CGameZombie::Notify(EventClientDisconnected e)
{
	GetWorld()->DelObj(e.client->m_obj);
    e.client->m_obj = 0;
}

void CGameZombie::Update(const float dt, const DWORD ticks)
{
	CGameObj* obj;
	OBJITER iter;

	for(iter = GetWorld()->ObjBegin();iter!=GetWorld()->ObjEnd();iter++)
	{
		obj = (CGameObj*)(*iter).second;

        obj->m_think.DoThink(GetWorld()->GetLeveltime());

		GetWorld()->ObjMove(obj, dt);
        if(obj->IsClient())
        {
            if(vec3_t(obj->GetVel().x, 0.0f, obj->GetVel().z).AbsSquared() > 
               100*lynxmath::EPSILON)
                obj->SetNextAnimation(obj->GetAnimationFromName("run"));
            else
                obj->SetNextAnimation(0);

            ProcessClientCmds((CGameObjPlayer*)obj, obj->GetClientID());
        }
	}
}

void CGameZombie::ProcessClientCmds(CGameObjPlayer* clientobj, int clientid)
{
    CClientInfo* client = GetServer()->GetClient(clientid);
    assert(client);
    bool bFire = false;

    std::vector<std::string>::iterator iter;
    for(iter = client->clcmdlist.begin();iter != client->clcmdlist.end();iter++)
    {
        if((*iter) == "+fire")
        {
            bFire = true;
        }
    }

    clientobj->CmdFire(bFire);

    client->clcmdlist.clear();
}
