#pragma once

#include <enet/enet.h>
#include "WorldClient.h"
#include "Observer.h"

class CClient : public CObserver
{
public:
	CClient(CWorldClient* world);
	~CClient(void);

	bool Connect(char* server, int port);
	void Shutdown();

	bool IsConnecting() { return m_isconnecting && m_server; }
	bool IsConnected() { return m_server && !m_isconnecting; }

	void Update(const float dt);

protected:
	void OnReceive(CStream* stream);

	void InputMouseMove(int dx, int dy);
	void InputCalcDir();
	CObj* GetLocalObj();

	void NotifyError(int error);

private:
	ENetHost* m_client;
	ENetPeer* m_server;
	bool m_isconnecting;

	// Input
	int m_forward;
	int m_backward;
	int m_strafe_left;
	int m_strafe_right;

	CWorldClient* m_world;
};
