#pragma once

#include <enet/enet.h>
#include <map>
#include "World.h"
#include "ClientInfo.h"
#include "Subject.h"
#include "Events.h"
#include "Stream.h"

#define CLIENTITER          std::map<int, CClientInfo*>::iterator

class CServer : public CSubject<EventNewClientConnected>,
                public CSubject<EventClientDisconnected>
{
public:
    CServer(CWorld* world);
    ~CServer(void);

    bool            Create(int port); // Start server on port
    void            Shutdown(); // Stop server

    void            Update(const float dt, const uint32_t ticks);

    CClientInfo*    GetClient(int id);
    int             GetClientCount() const;
    CLIENTITER      GetClientBegin() { return m_clientlist.begin(); }
    CLIENTITER      GetClientEnd() { return m_clientlist.end(); }

protected:
    bool SendWorldToClient(CClientInfo* client);
    void OnReceive(CStream* stream, CClientInfo* client);

    void UpdateHistoryBuffer(); // Alte HistoryBuffer Eintr�ge l�schen, kein Client ben�tigt mehr so eine alte Welt, oder die Welt ist zu alt und Client bekommt ein komplettes Update.
    void ClientHistoryACK(CClientInfo* client, uint32_t worldid); // Client best�tigt

private:
    ENetHost* m_server;
    std::map<int, CClientInfo*> m_clientlist;

    std::map<uint32_t, world_state_t> m_history; // World History Buffer. Ben�tigt f�r Quake 3 Network Modell bzw. differentielle Updates an Clients

    uint32_t m_lastupdate;
    CWorld* m_world;

    CStream m_stream; // damit buffer nicht jedesmal neu erstellt werden muss
};
