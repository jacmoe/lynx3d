#include "NetMsg.h"
#include "Server.h"
#include "ServerClient.h"
#include <algorithm> // remove and remove_if

#ifdef _DEBUG
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK,__FILE__, __LINE__)
#endif

CServer::CServer(CWorld* world)
{
    enet_initialize();
    m_server = NULL;
    m_lastupdate = 0;
    m_world = world;
    m_stream.Resize(MAX_SV_PACKETLEN);
}

CServer::~CServer(void)
{
    Shutdown();
    enet_deinitialize();
}

bool CServer::Create(int port)
{
    ENetAddress addr;

    addr.host = ENET_HOST_ANY;
    addr.port = port;

    // channellimit 5 (in theory only 1 channel is used, but lets play safe
    // here)
    m_server = enet_host_create(&addr, MAXCLIENTS, 5, 0, 0);
    if(!m_server)
        return false;

#ifdef USE_RANGE_ENCODER
    enet_host_compress_with_range_coder(m_server);
#endif

    return true;
}

void CServer::Shutdown()
{
    if(m_server)
    {
        enet_host_destroy(m_server);
        m_server = NULL;
    }

    std::map<int, CClientInfo*>::iterator iter;
    for(iter = m_clientlist.begin();iter!=m_clientlist.end();iter++)
        delete (*iter).second;
    m_clientlist.clear();
}

CClientInfo* CServer::GetClient(int id)
{
    CLIENTITER iter;
    iter = m_clientlist.find(id);
    if(iter == GetClientEnd())
        return NULL;
    return (*iter).second;
}

int CServer::GetClientCount() const
{
    return (int)m_clientlist.size();
}

void CServer::Update(const float dt, const uint32_t ticks)
{
    ENetEvent event;
    CClientInfo* clientinfo;
    std::map<int, CClientInfo*>::iterator iter;
    CStream stream;

    while(enet_host_service(m_server, &event, 0) > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            clientinfo = new CClientInfo(event.peer);
            // Fire Event
            {
            char hostname[64];
            enet_address_get_host_ip(&event.peer->address,
                                     hostname, sizeof(hostname));
            fprintf(stderr, "A new client connected from %s:%u.\n",
                    hostname,
                    event.peer->address.port);

            EventNewClientConnected e;
            e.client = clientinfo;
            CSubject<EventNewClientConnected>::NotifyAll(e);
            }

            event.peer->data = clientinfo;
            m_clientlist[clientinfo->GetID()] = clientinfo;
            break;

        case ENET_EVENT_TYPE_RECEIVE:
            stream.SetBuffer(event.packet->data,
                            (int)event.packet->dataLength,
                            (int)event.packet->dataLength);
            OnReceive(&stream, (CClientInfo*)event.peer->data);

            enet_packet_destroy (event.packet);

            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            clientinfo = (CClientInfo*)event.peer->data;
            assert(clientinfo);
            fprintf(stderr, "Client %i disconnected.\n", clientinfo->GetID());

            // Observer benachrichtigen
            {
            EventClientDisconnected e;
            e.client = clientinfo;
            CSubject<EventClientDisconnected>::NotifyAll(e);
            }

            // Client aus Client-Liste l�schen und Speicher freigeben
            iter = m_clientlist.find(clientinfo->GetID());
            assert(iter != m_clientlist.end());
            delete (*iter).second;
            m_clientlist.erase(iter);
            event.peer->data = NULL;
        }
    }

    if((ticks - m_lastupdate) > SERVER_UPDATETIME)
    {
        int sent = 0;
        std::map<int, CClientInfo*>::iterator iter;
        for(iter = m_clientlist.begin();iter!=m_clientlist.end();iter++)
        {
            if(SendWorldToClient((*iter).second))
                sent++;
        }

        m_lastupdate = ticks;
        if(sent > 0)
        {
            assert(m_history.find(m_world->GetWorldID()) == m_history.end());
            m_history[m_world->GetWorldID()] = m_world->GetWorldState();
        }
        UpdateHistoryBuffer();
    }
}

void CServer::OnReceive(CStream* stream, CClientInfo* client)
{
    uint8_t type;

    type = CNetMsg::ReadHeader(stream);
    switch(type)
    {
    case NET_MSG_CLIENT_CTRL:
        {
            uint32_t worldid;
            stream->ReadDWORD(&worldid);
            ClientHistoryACK(client, worldid);

            CObj* obj = m_world->GetObj(client->m_obj);
            assert(obj);
            vec3_t origin, vel;
            stream->ReadVec3(&origin);
            stream->ReadVec3(&vel);
            stream->ReadFloat(&client->lat);
            stream->ReadFloat(&client->lon);
            if((origin - obj->GetOrigin()).AbsSquared() < MAX_SV_CL_POS_DIFF)
            {
                obj->SetOrigin(origin);
            }
            else
            {
                fprintf(stderr, "SV: Reset client position\n");
            }
            obj->SetVel(vel);

            assert(client->clcmdlist.size() < 30);
            uint16_t cmdcount;
            stream->ReadWORD(&cmdcount);
            client->clcmdlist.clear();
            for(int i=0;i<cmdcount;i++)
            {
                std::string cmd;
                stream->ReadString(&cmd);
                client->clcmdlist.push_back(cmd);
            }
        }
        break;
    case NET_MSG_INVALID:
    default:
        assert(0);
    }


}

void CServer::UpdateHistoryBuffer()
{
    uint32_t lowestworldid = 0;
    CClientInfo* client;
    std::map<uint32_t, world_state_t>::iterator worlditer;
    std::map<int, CClientInfo*>::iterator clientiter;
    uint32_t worldtime;
    uint32_t curtime = m_world->GetLeveltime();

    for(clientiter = m_clientlist.begin();clientiter!=m_clientlist.end();clientiter++)
    {
        client = (*clientiter).second;

        if(client->worldidACK == 0) // dieser client hat keine bekannte welt
            continue;

        worlditer = m_history.find(client->worldidACK);
        if(worlditer == m_history.end())
        {
            //assert(0);
            client->worldidACK = 0;
            fprintf(stderr, "Client last known world is not in history buffer\n");
            continue;
        }
        worldtime = ((*worlditer).second).leveltime;
        if(curtime - worldtime > SERVER_MAX_WORLD_AGE)
        {
            fprintf(stderr, "Client world is too old (%i ms)\n", (int)(curtime - worldtime));
            client->worldidACK = 0;
            continue;
        }
        if((lowestworldid == 0 || client->worldidACK < lowestworldid) && client->worldidACK > 0)
        {
            lowestworldid = client->worldidACK;
        }
    }

    for(worlditer = m_history.begin();worlditer != m_history.end();)
    {
        worldtime = ((*worlditer).second).leveltime;
        if(curtime - worldtime > SERVER_MAX_WORLD_AGE ||
            ((*worlditer).second).worldid < lowestworldid)
        {
            //worlditer = m_history.erase(worlditer);
            m_history.erase(worlditer++);
        }
        else
        {
            ++worlditer;
        }
    }

    assert(m_history.size() < MAX_WORLD_BACKLOG);
    if(m_history.size() >= MAX_WORLD_BACKLOG)
    {
        fprintf(stderr, "Server History Buffer too large. Reset History Buffer.\n");
        m_history.clear();
    }
}

void CServer::ClientHistoryACK(CClientInfo* client, uint32_t worldid)
{
    if(client->worldidACK < worldid)
        client->worldidACK = worldid;
}

bool CServer::SendWorldToClient(CClientInfo* client)
{
    ENetPacket* packet;
    int localobj = client->m_obj;
    m_stream.ResetWritePosition();

    CNetMsg::WriteHeader(&m_stream, NET_MSG_SERIALIZE_WORLD); // Writing Header
    m_stream.WriteWORD((uint16_t)localobj);
    client->hud.Serialize(true, &m_stream, NULL);

    std::map<uint32_t, world_state_t>::iterator iter;
    iter = m_history.find(client->worldidACK);
    if(iter == m_history.end())
    {
        m_world->Serialize(true, &m_stream, NULL);
        // fprintf(stderr, "NET: Full update MTU: %i Bytes to be send: %i\n",
        //         client->GetPeer()->mtu,
        //         m_stream.GetBytesWritten());
    }
    else
    {
        world_state_t diffstate = (*iter).second;
        if(!m_world->Serialize(true, &m_stream, &diffstate))
        {
            // Seit dem letzten best�tigtem Update vom Client hat sich nichts getan.
            client->worldidACK = m_world->GetWorldID();
            return true; // client ben�tigt kein update
        }
        // fprintf(stderr, "NET: Half update MTU: %i Bytes to be send: %i\n",
        //         client->GetPeer()->mtu,
        //         m_stream.GetBytesWritten());
    }

    // if(client->GetPeer()->mtu < m_stream.GetBytesWritten())
    // {
    //     fprintf(stderr, "NET: Packet fragmentation MTU: %i Bytes to be send: %i\n",
    //             client->GetPeer()->mtu,
    //             m_stream.GetBytesWritten());
    // }
    packet = enet_packet_create(m_stream.GetBuffer(),
                                m_stream.GetBytesWritten(),
                                0);
    assert(packet);
    if(!packet)
        return false;

    return enet_peer_send(client->GetPeer(), 0, packet) == 0;
}

