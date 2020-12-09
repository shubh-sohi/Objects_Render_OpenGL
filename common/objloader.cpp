#include <vector>
#include <stdio.h>
#include <string>
#include <cstring>
#include <algorithm>
#include <glm/glm.hpp>

#include "objloader.hpp"


// Very, VERY simple OBJ loader.
// Here is a short list of features a real function would provide : 
// - Binary files. Reading a model should be just a few memcpy's away, not parsing a file at runtime. In short : OBJ is not very great.
// - Animations & bones (includes bones weights)
// - Multiple UVs
// - All attributes should be optional, not "forced"
// - More stable. Change a line in the OBJ file and it crashes.
// - More secure. Change another line and you can inject code.
// - Loading from memory, stream, etc


bool getNextLine(FILE* file, char* line) {
    char word[128];
    while( 1 ) {
        
        // grab line
        fgets(line, 1024, file);
        // grab first word in line
        int res = sscanf(line, "%s", word);
        if (res == EOF) {
            return false; // EOF found
        } else if ( strcmp( word, "#" ) == 0 ) {
            // found comment, ignore and read next line
            continue;
		} else {
            return true;
        }
    }
}


bool loadModels(const char* path, std::vector<Model>& out_models) {
	printf("Loading MODELS file %s...\n", path);
    
	FILE * file = fopen(path, "r");
	if( file == NULL ){
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
		getchar();
		return false;
	}
    
    char line[1024];
    out_models.clear();
    
    int numModels = 0;
    getNextLine(file, line);
    sscanf(line, "%d", &numModels);
    out_models.resize(numModels);
    
//    float sx,sy,sz, rx,ry,rz,ra, tx,ty,tz;
//    float ar,ag,ab, dr,dg,db, sr,sg,sb,ss;
    char str[1024];
    for (int i = 0; i < numModels; ++i) {
        Model m;
        // read obj file name
        getNextLine(file, line);
        sscanf(line, "%s\n", str);
        m.objFilename.assign(str);
        
        // read transformation
        getNextLine(file, line);
        sscanf(line, "%f %f %f %f %f %f %f %f %f %f\n",
               &m.sx, &m.sy, &m.sz,
               &m.rx, &m.ry, &m.rz, &m.ra,
               &m.tx, &m.ty, &m.tz );

        // read material
        getNextLine(file, line);
        sscanf(line, "%f %f %f %f %f %f %f %f %f %f\n",
               &m.ar, &m.ag, &m.ab,
               &m.dr, &m.dg, &m.db,
               &m.sr, &m.sg, &m.sb, &m.ss );

        getNextLine(file, line);
        sscanf(line, "%s\n", str);
        m.textureFilename.assign(str);
        
        out_models[i]=m;
    }
	return true;
}

//
// load an .obj file:
// - first read raw data from the file with loadOBJ_indexed
// - then unroll the indices and return full lists of vertices, uvs, and normals
//
bool loadOBJ(
             const char * path,
             std::vector<glm::vec3> & out_vertices,
             std::vector<glm::vec2> & out_uvs,
             std::vector<glm::vec3> & out_normals
             ){
	printf("Loading OBJ file %s...\n", path);
    
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;
    std::vector<glm::ivec3> temp_vertexIndices;
    std::vector<glm::ivec3> temp_uvIndices;
    std::vector<glm::ivec3> temp_normalIndices;
    
    loadOBJ_indexed_modified(path, temp_vertices, temp_uvs, temp_normals, temp_vertexIndices, temp_uvIndices, temp_normalIndices);
    
    // Unroll indices and return expanded buffers of vertex positions, uvs, and normals
	// For each vertex of each triangle
	for( unsigned int vi=0; vi<temp_vertexIndices.size(); vi++ ){
        for (unsigned int i=0; i < 3; i++ ){
            
            // Get the indices of its attributes
            unsigned int vertexIndex = temp_vertexIndices[vi][i];
            // Get the attributes thanks to the index
            glm::vec3 vertex = temp_vertices[ vertexIndex ];
            // Put the attributes in buffers
            out_vertices.push_back(vertex);

            // same for uvs if we have them
            if (temp_uvs.size() > 0) {
                unsigned int uvIndex = temp_uvIndices[vi][i];
                glm::vec2 uv = temp_uvs[ uvIndex ];
                out_uvs.push_back(uv);
            }
            
            // same for normals if we have them
            if (temp_normals.size() > 0) {
                unsigned int normalIndex = temp_normalIndices[vi][i];
                glm::vec3 normal = temp_normals[ normalIndex ];
                out_normals .push_back(normal);
            }
        }
	}
    
	return true;
}

//
// load an .obj file with indices:
// - read vertices, uvs, normals, and associated indices from a .obj file
//
bool loadOBJ_indexed(const char * path,
                     std::vector<glm::vec3> & vertices,
                     std::vector<glm::vec2> & uvs,
                     std::vector<glm::vec3> & normals,
                     std::vector<glm::ivec3> & vertexIndices,
                     std::vector<glm::ivec3> & uvIndices,
                     std::vector<glm::ivec3> & normalIndices
                     ){
	printf("Loading OBJ file indexed %s...\n", path);

	FILE * file = fopen(path, "r");
	if( file == NULL ){
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
		getchar();
		return false;
	}

    int vCnt = 0;
    int uvCnt = 0;
    int nrmlCnt = 0;
    int faceCnt = 0;

	while( 1 ){

		char lineHeader[128];

		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		// else : parse lineHeader
		
		if ( strcmp( lineHeader, "v" ) == 0 ){
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
			vertices.push_back(vertex);
            vCnt++;
		}else if ( strcmp( lineHeader, "vt" ) == 0 ){
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y );
			uvs.push_back(uv);
            uvCnt++;
		}else if ( strcmp( lineHeader, "vn" ) == 0 ){
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
			normals.push_back(normal);
            nrmlCnt++;
		}else if ( strcmp( lineHeader, "f" ) == 0 ){
            faceCnt++;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            if (uvCnt > 0 && nrmlCnt > 0) {
                int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
                if (matches != 9){
                    printf("LoadOBJ parser failed.\n");
                    return false;
                }
            }
            else if (uvCnt > 0) {
                int matches = fscanf(file, "%d/%d %d/%d %d/%d\n", &vertexIndex[0], &uvIndex[0], &vertexIndex[1], &uvIndex[1], &vertexIndex[2], &uvIndex[2] );
                if (matches != 6){
                    printf("LoadOBJ parser failed.\n");
                    return false;
                }
            }
            else if (nrmlCnt > 0) {
                int matches = fscanf(file, "%d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2] );
                if (matches != 6){
                    printf("LoadOBJ parser failed.\n");
                    return false;
                }
            }
            else {
                int matches = fscanf(file, "%d %d %d\n", &vertexIndex[0], &vertexIndex[1], &vertexIndex[2]);
                if (matches != 3) {
                    printf("LoadOBJ parser failed.\n");
                    return false;
                }
            }
            
            int off = 1; // index offset: assume obj file indexes start at 1
            vertexIndices.push_back(glm::ivec3(vertexIndex[0]-off, vertexIndex[1]-off, vertexIndex[2]-off));
            if (uvCnt > 0)
                uvIndices.push_back(glm::ivec3(uvIndex[0]-off, uvIndex[1]-off, uvIndex[2]-off));
            if (nrmlCnt > 0)
                normalIndices.push_back(glm::ivec3(normalIndex[0]-off, normalIndex[1]-off, normalIndex[2]-off));
            
		}else{
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

    }
	return true;
}

//
// Determine if a character is visible
//
bool invisibleChar(char c){
    return (c <= 32 || c > 120);
}

//
// load an .obj file:
// - first read raw data from the file with loadOBJ_indexed
// - then unroll the indices and return full lists of vertices, uvs, and normals
// This function is modified to ceck the number of vertices specified on a face line
bool loadOBJ_indexed_modified(const char * path,
                     std::vector<glm::vec3> & vertices,
                     std::vector<glm::vec2> & uvs,
                     std::vector<glm::vec3> & normals,
                     std::vector<glm::ivec3> & vertexIndices,
                     std::vector<glm::ivec3> & uvIndices,
                     std::vector<glm::ivec3> & normalIndices
                     ){
    printf("Loading OBJ file indexed %s...\n", path);

    FILE * file = fopen(path, "r");
    if( file == NULL ){
        printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
        getchar();
        return false;
    }   

    int vCnt = 0;
    int uvCnt = 0;
    int nrmlCnt = 0;
    int faceCnt = 0;
    std::string s;
    while( 1 ){

        char lineHeader[128];
        char faceLine[128];
        // read the first word of the line
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
            break; // EOF = End Of File. Quit the loop.

        // else : parse lineHeader
    
        if ( strcmp( lineHeader, "v" ) == 0 ){
            glm::vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
            vertices.push_back(vertex);
            vCnt++;
        }else if ( strcmp( lineHeader, "vt" ) == 0 ){
            glm::vec2 uv; 
            fscanf(file, "%f %f\n", &uv.x, &uv.y );
            uvs.push_back(uv);
            uvCnt++;
        }else if ( strcmp( lineHeader, "vn" ) == 0 ){
            glm::vec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
            normals.push_back(normal);
            nrmlCnt++;
        }else if ( strcmp( lineHeader, "f" ) == 0 ){
            faceCnt++;
            unsigned int vertexIndex[4], uvIndex[4], normalIndex[4];
            fgets(faceLine, 128, file);
            s.assign(faceLine);
    
            // Remove invisible characters from front
            while(invisibleChar(s[0])){
                s.erase(0,1);
            }

            // Remove invisible character from end
            while(invisibleChar(s[s.length()-1])){
                s.erase(s.length()-1,1);
            }
            // Check number of spaces: if there is 3 then 4 vertices are specified, else probably just 3
            int numSpaces = std::count(s.begin(), s.end(), ' ');
            if (uvCnt > 0 && nrmlCnt > 0) {
                if (numSpaces == 3){
                    int matches = sscanf(faceLine, "%d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2], &vertexIndex[3], &uvIndex[3], &normalIndex[3] );
                    if (matches != 12){
                        printf("LoadOBJ parser failed.\n");
                        printf("%s",faceLine);
                        return false;
                    }
                }else{
                    int matches = sscanf(faceLine, " %d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
                    if (matches != 9){
                        printf("LoadOBJ parser failed.\n");
                        printf("%s",faceLine);
                        return false;
                    }
                }
            }
            else if (uvCnt > 0) {
                if (numSpaces == 3){
                    int matches = sscanf(faceLine, "%d/%d %d/%d %d/%d %d/%d\n", &vertexIndex[0], &uvIndex[0], &vertexIndex[1], &uvIndex[1], &vertexIndex[2], &uvIndex[2],&vertexIndex[3], &uvIndex[3]);
                    if (matches != 8){
                        printf("LoadOBJ parser failed.\n");
                        printf("%s",faceLine);
                        return false;
                    }
                }else{
                    int matches = sscanf(faceLine, "%d/%d %d/%d %d/%d\n", &vertexIndex[0], &uvIndex[0], &vertexIndex[1], &uvIndex[1], &vertexIndex[2], &uvIndex[2] );
                    if (matches != 6){
                        printf("LoadOBJ parser failed.\n");
                        printf("%s",faceLine);
                        return false;

                    }
                }
            }
            else if (nrmlCnt > 0) {
                if (numSpaces == 3){
                    int matches = sscanf(faceLine, "%d//%d %d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2],&vertexIndex[3], &uvIndex[3]);
                    if (matches != 8){
                        printf("LoadOBJ parser failed.\n");
                        printf("%s",faceLine);
                        return false;
                    }
                    }else{
                    int matches = sscanf(faceLine, "%d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2] );
                    if (matches != 6){
                        printf("LoadOBJ parser failed.\n");
                        printf("%s",faceLine);
                        return false;
                    }
                }
            }
            else {
                if (numSpaces == 3){
                    int matches = sscanf(faceLine, "%d %d %d %d\n", &vertexIndex[0], &vertexIndex[1], &vertexIndex[2], &vertexIndex[3]);
                    if (matches != 4) {
                        printf("LoadOBJ parser failed.\n");
                        printf("%s",faceLine);
                        return false;
                    }
                }else{
                    int matches = sscanf(faceLine, "%d %d %d\n", &vertexIndex[0], &vertexIndex[1], &vertexIndex[2]);
                    if (matches != 3) {
                        printf("LoadOBJ parser failed.\n");
                        printf("%s",faceLine);
                        return false;
                    }
                }
            }

            int off = 1; // index offset: assume obj file indexes start at 1
            // If there were 3 spaces found add 2 triangles, otherwise add one
            if (numSpaces == 3){
                vertexIndices.push_back(glm::ivec3(vertexIndex[0]-off, vertexIndex[1]-off, vertexIndex[2]-off));
                vertexIndices.push_back(glm::ivec3(vertexIndex[0]-off, vertexIndex[2]-off, vertexIndex[3]-off));
                if (uvCnt > 0)
                    uvIndices.push_back(glm::ivec3(uvIndex[0]-off, uvIndex[1]-off, uvIndex[2]-off));
                    uvIndices.push_back(glm::ivec3(uvIndex[0]-off, uvIndex[2]-off, uvIndex[3]-off));
                if (nrmlCnt > 0)
                    normalIndices.push_back(glm::ivec3(normalIndex[0]-off, normalIndex[1]-off, normalIndex[2]-off));
                    normalIndices.push_back(glm::ivec3(normalIndex[0]-off, normalIndex[2]-off, normalIndex[3]-off));

            }else{
                vertexIndices.push_back(glm::ivec3(vertexIndex[0]-off, vertexIndex[1]-off, vertexIndex[2]-off));
                if (uvCnt > 0)
                    uvIndices.push_back(glm::ivec3(uvIndex[0]-off, uvIndex[1]-off, uvIndex[2]-off));
                if (nrmlCnt > 0)
                    normalIndices.push_back(glm::ivec3(normalIndex[0]-off, normalIndex[1]-off, normalIndex[2]-off));

            }
        }else{
            // Probably a comment, eat up the rest of the line
            char stupidBuffer[1000];
            fgets(stupidBuffer, 1000, file);
        }
    }
    return true;
}
                                             


#ifdef USE_ASSIMP // don't use this #define, it's only for me (it AssImp fails to compile on your machine, at least all the other tutorials still work)

// Include AssImp
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

bool loadAssImp(
	const char * path, 
	std::vector<unsigned short> & indices,
	std::vector<glm::vec3> & vertices,
	std::vector<glm::vec2> & uvs,
	std::vector<glm::vec3> & normals
){

	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(path, 0/*aiProcess_JoinIdenticalVertices | aiProcess_SortByPType*/);
	if( !scene) {
		fprintf( stderr, importer.GetErrorString());
		getchar();
		return false;
	}
	const aiMesh* mesh = scene->mMeshes[0]; // In this simple example code we always use the 1rst mesh (in OBJ files there is often only one anyway)

	// Fill vertices positions
	vertices.reserve(mesh->mNumVertices);
	for(unsigned int i=0; i<mesh->mNumVertices; i++){
		aiVector3D pos = mesh->mVertices[i];
		vertices.push_back(glm::vec3(pos.x, pos.y, pos.z));
	}

	// Fill vertices texture coordinates
	uvs.reserve(mesh->mNumVertices);
	for(unsigned int i=0; i<mesh->mNumVertices; i++){
		aiVector3D UVW = mesh->mTextureCoords[0][i]; // Assume only 1 set of UV coords; AssImp supports 8 UV sets.
		uvs.push_back(glm::vec2(UVW.x, UVW.y));
	}

	// Fill vertices normals
	normals.reserve(mesh->mNumVertices);
	for(unsigned int i=0; i<mesh->mNumVertices; i++){
		aiVector3D n = mesh->mNormals[i];
		normals.push_back(glm::vec3(n.x, n.y, n.z));
	}


	// Fill face indices
	indices.reserve(3*mesh->mNumFaces);
	for (unsigned int i=0; i<mesh->mNumFaces; i++){
		// Assume the model has only triangles.
		indices.push_back(mesh->mFaces[i].mIndices[0]);
		indices.push_back(mesh->mFaces[i].mIndices[1]);
		indices.push_back(mesh->mFaces[i].mIndices[2]);
	}
	
	// The "scene" pointer will be deleted automatically by "importer"

}

#endif
