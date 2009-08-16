#pragma once

#include "WorldClient.h"

class CRenderer
{
public:
	CRenderer(CWorldClient* world);
	~CRenderer(void);

	bool Init(int width, int height, int bpp, int fullscreen);
	void Shutdown();

	void Update(const float dt, const DWORD ticks);

	// Public Stats (FIXME)
	int stat_obj_visible;
	int stat_obj_hidden;
    int stat_bsp_leafs_visited;

protected:
	void UpdatePerspective();

private:
	int m_width, m_height;
	CWorldClient* m_world;
};