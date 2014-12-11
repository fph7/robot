/*
Modified for use with robot model by Frank P Haecker
*/

/*Original File: mini_geometry.h
   Author: T.J. Jankun-Kelly <tjk@cse.msstate.edu>
   Geometry for the Mini Cooper model.

The Cooper model is formed of various groups. Each group has a name (the
"part") that determines which part it belongs to. For example, the "Roof" part
posses geometry that belongs to the roof of the car. To get the
starting and ending indices of the faces belonging to the part, use the groups
function; this can be used to render only that face (or to walk over all):
    MiniGeometry mini();
    <load vertex buffer and index buffer from mini.vertexdata & mini.indices>
    list<PartEntry> groups = mini.groups();
    list<PartEntry>::const_iterator it;
    for(it = groups.begin(); it != groups.end(); ++it)
    {
        string name = entry.name;
        int offset = sizeof(GLushort) * 3 * entry.start;
        int count = 3 * (entry.end - entry.start);
        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, offsset);
    }
*/
#include <string>
#include <list>
#include <fstream>

#ifndef __MINI__
#define __MINI__

// Stores info for a part: Its name, the starting index of faces that belong
// to it, and their ending index
struct PartEntry
{
    std::string name;
    int start;
    int end;
};

// The actual geometry class. Reads data for you.
class MiniGeometry
{
    private:
        int _numVertices;           // Total num of vertices and attributes
                                    // Note: There are 3 entries per vertex:
                                    //       xyz for position
        float *data;                // Float to hold the data
                                    // 3*numVertice elements
        int _numFaces;              // Number of faces
        unsigned short *_indices;   // Indices of vertex data per face. 
                                    // 3*numFaces elemets
        std::list<PartEntry> _groups;    // Part names and start/end indices
        
        
    public:   
        MiniGeometry()
        {
            /* Read in geometry
               Format:
                numVertices
                vx vy vz
                ...
                numFaces
                vertexIndex0 vertexIndex1 vertexIndex2
                ...
                numGroups
                faceStart faceEnd name 
            */
            std::ifstream geom("robot_geometry.txt");
    
            // Store the vertex + attribute data in one array. Easier for
            // copying to the GPU later
            geom >> _numVertices;
            data = new float[3*_numVertices];
            for(int i = 0; i < _numVertices; ++i)
            {
                for(int j = 0; j < 3; ++j)
                {
                    geom >> data[3*i+j];
                }
            }
                    
            // Store the face indices in one array of unsigned shorts for easier
            // copying to the GPU later
            geom >> _numFaces;
            _indices = new unsigned short[4*_numFaces];
            for(int i = 0; i < _numFaces; ++i)
            {
                for(int j = 0; j < 4; ++j)
                {
                    geom >> _indices[4*i+j];
                }
            }
                    
            // Store groups (and names) in and ordered dictionary
            int numParts;
            geom >> numParts;
            for(int i = 0; i < numParts; ++i)
            {
                PartEntry entry;
                geom >> entry.start >> entry.end;
                geom.get(); // clear the space
                std::getline(geom, entry.name);
                _groups.push_back(entry);
            }
            
            geom.close();
        }
        
        MiniGeometry(const MiniGeometry& other)
        {
            data = 0;
            _indices = 0;
            *this = other;
        }
        
        virtual ~MiniGeometry()
        {
            delete [] data;
            delete [] _indices;
        }
        
        MiniGeometry& operator=(const MiniGeometry& rhs)
        {
            if(&rhs != this)
            {
                if(data != 0) delete [] data;
                if(_indices != 0) delete [] _indices;

                _numVertices = rhs._numVertices;
                data = new float[3*_numVertices];
                for(int i = 0; i < _numVertices; ++i)
                {
                    for(int j = 0; j < 3; ++j)
                    {
                        data[3*i+j] = rhs.data[3*i+j];
                    }
                }

                _numFaces = rhs._numFaces;
                _indices = new unsigned short[4*_numFaces];
                for(int i = 0; i < _numFaces; ++i)
                {
                    for(int j = 0; j < 4; ++j)
                    {
                        _indices[4*i+j] = rhs._indices[4*i+j];
                    }
                }
                
                _groups = rhs._groups;
            }
            return *this;
        }

        // Data access members. 
        int numVertices() const {return _numVertices;}
        float* vertexdata() {return data;}
        int numFaces() const {return _numFaces;}
        unsigned short const* indices() const {return _indices;}
        std::list<PartEntry> groups() const {return _groups;}
        std::list<std::string> parts() const
        {
            std::list<std::string> names;
            std::list<PartEntry>::const_iterator it;
            for(it = _groups.begin(); it != _groups.end(); ++it)
            {
                names.push_back(it->name);
            }
            
            return names;
        }
};

#endif
