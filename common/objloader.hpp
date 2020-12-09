#ifndef OBJLOADER_H
#define OBJLOADER_H
#include <string>

struct Model {
    // model files
    std::string objFilename;
    std::string textureFilename;
    
    // model transformation
    //    float scale[3];
    //    float axisAngleRot[4];
    //    float translation[3];
    float sx,sy,sz, rx,ry,rz,ra, tx,ty,tz;
    
    // material properties
    //    float ambient[3];
    //    float diffuse[3];
    //    float specular[3];
    //    float shininess;
    float ar,ag,ab, dr,dg,db, sr,sg,sb,ss;
    
};

bool loadModels(
    const char* path,
    std::vector<Model>& out_models
);


bool loadOBJ(
	const char * path, 
	std::vector<glm::vec3> & out_vertices, 
	std::vector<glm::vec2> & out_uvs, 
	std::vector<glm::vec3> & out_normals
);

bool loadOBJ_indexed(
     const char * path,
     std::vector<glm::vec3> & vertices,
     std::vector<glm::vec2> & uvs,
     std::vector<glm::vec3> & normals,
     std::vector<glm::ivec3> & vertexIndices,
     std::vector<glm::ivec3> & uvIndices,
     std::vector<glm::ivec3> & normalIndices
);

bool invisibleChar(char c);

bool loadOBJ_indexed_modified(
     const char * path,
     std::vector<glm::vec3> & vertices,
     std::vector<glm::vec2> & uvs,
     std::vector<glm::vec3> & normals,
     std::vector<glm::ivec3> & vertexIndices,
     std::vector<glm::ivec3> & uvIndices,
     std::vector<glm::ivec3> & normalIndices
);

bool loadAssImp(
	const char * path, 
	std::vector<unsigned short> & indices,
	std::vector<glm::vec3> & vertices,
	std::vector<glm::vec2> & uvs,
	std::vector<glm::vec3> & normals
);

#endif
