#pragma once
#include <enet/enet.h>
#include <vector>
#include <string>

class CClientInfo
{
public:
	CClientInfo(ENetPeer* peer)
	{
		m_id = ++m_idpool;
		m_peer = peer;
		m_obj = 0;
		worldidACK = 0;
        lat = lon = 0.0f;
	}

	int GetID() const { return m_id; }
	ENetPeer* GetPeer() { return m_peer; }
	
    int m_obj; // client obj in world	
    DWORD worldidACK; // Last ACK'd world from client (build diffs to this world)

    // Client Input
    std::vector<std::string> clcmdlist;
    float lat, lon; // mouse lat and lon

private:
	int m_id;
	ENetPeer* m_peer;
	static int m_idpool;
};