#pragma once

#include "lynx.h"
#include "math/vec3.h"
#include "math/plane.h"
#include "math/quaternion.h"
#include <vector>
#include "ResourceManager.h"

/*
    CBSPTree verwaltet die Level Geometrie und wird
    f�r die Kollisionserkennung und Darstellung genutzt.
    
    BSP steht f�r bin�re Raumteilung und erlaubt Operationen in O(log n)
    Zeit.
    Dieser Baum ist Leaf-based, d.h. alle Polygone stehen in
    den Bl�ttern vom Baum und formen konvexe Subr�ume
 */

class CBSPTree;

#define MAX_TRACE_DIST      99999.999f

struct bsp_sphere_trace_t
{
	vec3_t	start; // start point
    vec3_t  dir; // end point = start + dir
    float   radius;
	float	f; // impact = start + f*dir
	plane_t	p; // impact plane
};

struct bsp_poly_t
{
	bsp_poly_t() { splitmarker = false; colormarker = 0; }
	std::vector<int> vertices; // Eckpunkte
	std::vector<int> normals; // Normalen Vektoren
	std::vector<int> texcoords; // UV Koordinaten
	plane_t	plane; // Ebene von Polygon
    int texture; // Aktuelle Textur auf Polygon
	bool splitmarker; // true if this poly was used to split the space
	int colormarker; // Zu Testzwecken, ob das Polygon eingef�rbt werden soll. FIXME: entfernen, wenn alles klappt

	int Size() // vertex count
	{
		assert(vertices.size() == normals.size());
		return (int)vertices.size();
	}
	void Clear()
	{
		vertices.clear();
		normals.clear();
		texcoords.clear();
    }
    // Hilfsfunktionen zum Pr�fen Geradenschnitten mit der Polygon Geometrie
	bool GetIntersectionPoint(const vec3_t& p, const vec3_t& v, float* f, const CBSPTree* tree, const float offset = 0) const; // offset = plane offset
    bool GetEdgeIntersection(const vec3_t& start, const vec3_t& end, float* f, const float radius, vec3_t* normal, vec3_t* hitpoint, const CBSPTree* tree) const; // radius = edge radius
    bool GetVertexIntersection(const vec3_t& start, const vec3_t& end, float* f, const float radius, vec3_t* normal, vec3_t* hitpoint, const CBSPTree* tree) const;

    vec3_t GetNormal(CBSPTree* tree); // not unit length
	bool IsPlanar(CBSPTree* tree); // Pr�fen ob Polygon eben ist
	void GeneratePlanes(CBSPTree* tree); // Erzeugt den Normalenvektor der Fl�che
};

struct spawn_point_t
{
    vec3_t origin;
    quaternion_t rot;
};

enum polyplane_t {	POLYPLANE_SPLIT = 0,
					POLYPLANE_BACK = 1, 
					POLYPLANE_FRONT = 2, 
					POLYPLANE_COPLANAR = 3
					}; // if you change the order, change TestPolygon too

class CBSPTree
{
public:
	CBSPTree(void);
	~CBSPTree(void);

	bool		Load(std::string file, CResourceManager* resman); // L�dt den Level aus Wavefront .obj Dateien. Texturen werden �ber den ResourceManager geladen
	void		Unload();
	std::string GetFilename() const; // Aktuell geladener Pfad zu Level
	void		GetLeftRightScore(int* left, int* right) const; // Anzahl der Knoten im linken und rechten Bereich. N�tzlich um zu sehen, ob der Baum gut balanciert ist

	class CBSPNode // Hilfsklasse von BSPTree, stellt einen Knoten im Baum dar
	{
	public:
		~CBSPNode();
		CBSPNode(CBSPTree* tree, 
				 std::vector<bsp_poly_t>& polygons);

		union
		{
			struct
			{
				CBSPNode* front;
				CBSPNode* back;
			};
			CBSPNode* child[2];
		};

		void					CalculateSphere(CBSPTree* tree, std::vector<bsp_poly_t>& polygons); // Bounding Sphere f�r diesen Knoten berechnen
		bool					IsLeaf() const { return !front && !back; } // Ist Knoten ein Blatt?

		plane_t					plane; // Teilungsbene
		float					sphere; // Sphere Radius
		vec3_t					sphere_origin; // Sphere origin
		std::vector<bsp_poly_t> polylist; // Liste der Polygone, ist nur bei Bl�ttern gef�llt
		int						marker; // Markierer f�r Testzwecke. Um z.B. zu sehen wie viele Knoten bei einem Trace ber�hrt wurden
	};

	std::vector<vec3_t>		    m_vertices; // Vertexvektor
	std::vector<vec3_t>		    m_normals; // Normalenvektor
	std::vector<vec3_t>		    m_texcoords; // FIXME vec2_t w�rde reichen
	std::vector<bsp_poly_t>     m_polylist; // Vektor f�r Polygone
	CBSPNode*				    m_root; // Anfangsknoten

    void		TraceRay(const vec3_t& start, const vec3_t& dir, float* f) const;
    void		TraceSphere(bsp_sphere_trace_t* trace) const;
	void		ClearMarks(CBSPNode* node);
	void		MarkLeaf(const vec3_t& pos, float radius, CBSPNode* node);
	CBSPNode*	GetLeaf(const vec3_t& pos);
	int			GetLeafCount() const { return m_leafcount; }

    spawn_point_t GetRandomSpawnPoint() const;

protected:
    void		TraceSphere(bsp_sphere_trace_t* trace, const CBSPNode* node) const;
	void		TraceRay(const vec3_t& start, const vec3_t& dir, float* f, const CBSPNode* node) const; // Pr�fen, wo ein Strahl die Levelgeometrie trifft.

private:
	int			m_nodecount; // increased by every CBSPNode constructor
	int			m_leafcount;
	bool		m_outofmem; // set to true by CBSPNode constructor, if no memory for further child nodes is available
	std::string m_filename;

    // Spawn Point
    std::vector<spawn_point_t> m_spawnpoints; // Spawnpoints from level

	// Helper functions for the CBSPNode constructor to create the BSP tree
	polyplane_t TestPolygon(bsp_poly_t& poly, plane_t& plane);
	int			SearchSplitPolygon(std::vector<bsp_poly_t>& polygons);
	void		SplitPolygon(bsp_poly_t& polyin, plane_t& plane, 
							 bsp_poly_t& polyfront, bsp_poly_t& polyback);
	bool		IsConvexSet(std::vector<bsp_poly_t>& polygons);

	void		GetLeftRightScore(int* left, int* right, CBSPNode* node) const;
	CBSPNode*	GetLeaf(const vec3_t& pos, CBSPNode* node);
};
