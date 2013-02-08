#include "KoRE/SceneLoader.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "KoRE/MeshLoader.h"

kore::SceneLoader* kore::SceneLoader::getInstance(){
  static SceneLoader instance;
  return &instance;
}

kore::SceneLoader::SceneLoader() {
}

kore::SceneLoader::~SceneLoader() {
}

void kore::SceneLoader::loadScene(const std::string& szScenePath,
                                                SceneNodePtr parent) {
  const aiScene* pAiScene =
    _aiImporter.ReadFile(szScenePath,
                         aiProcess_JoinIdenticalVertices |
                         aiProcess_Triangulate);

  if (!pAiScene) {
    Log::getInstance()->write("[ERROR] Scene-file could not be loaded: %s",
      szScenePath.c_str());
    return;
  }
  //pAiScene->mRootNode->
}

void kore::SceneLoader::loadRessources( const std::string& szScenePath )
{
  const aiScene* pAiScene =
    _aiImporter.ReadFile(szScenePath,
    aiProcess_JoinIdenticalVertices |
    aiProcess_Triangulate);

  if (!pAiScene) {
    Log::getInstance()->write("[ERROR] Ressources could not be loaded: %s",
      szScenePath.c_str());
    return;
  }
  
  if (pAiScene->HasMeshes()) {
    for(uint i = 0; i>pAiScene->mNumMeshes; ++i) {
     loadMesh(pAiScene, i, true);
    }
  }
  if (pAiScene->HasTextures()) {
    // load textures as ressources with texture loader;
  }
}

kore::MeshPtr
  kore::SceneLoader::loadMesh(const aiScene* pAiScene,
  const uint uMeshIdx,
  const bool bUseBuffers) {
    kore::MeshPtr pMesh(new kore::Mesh);
    aiMesh* pAiMesh = pAiScene->mMeshes[ 0 ];
    pMesh->_numVertices = pAiMesh->mNumVertices;

    // TODO(dlazarek): Make more flexible here:
    pMesh->_primitiveType = GL_TRIANGLES;

    pMesh->_name = pAiMesh->mName.C_Str();

    if (pAiMesh->HasPositions()) {
      loadVertexPositions(pAiMesh, pMesh);
    }

    if (pAiMesh->HasNormals()) {
      loadVertexNormals(pAiMesh, pMesh);
    }

    if (pAiMesh->HasTangentsAndBitangents()) {
      loadVertexTangents(pAiMesh, pMesh);
    }

    // Load all texture coord-sets
    unsigned int iUVset = 0;
    while (pAiMesh->HasTextureCoords(iUVset++)) {
      loadVertexTextureCoords(pAiMesh, pMesh, iUVset);
    }

    // Load all vertex color sets
    unsigned int iColorSet = 0;
    while (pAiMesh->HasVertexColors(iColorSet++)) {
      loadVertexColors(pAiMesh, pMesh, iColorSet);
    }

    if (pAiMesh->HasFaces()) {
      loadFaceIndices(pAiMesh, pMesh);
    }

    if (bUseBuffers) {
      pMesh->createAttributeBuffers(BUFFERTYPE_INTERLEAVED);
    }

    return pMesh;
}

void kore::SceneLoader::
  loadVertexPositions(const aiMesh* pAiMesh,
  kore::MeshPtr& pMesh ) {
    unsigned int allocSize = pAiMesh->mNumVertices * 3 * 4;
    void* pVertexData = malloc(allocSize);
    memcpy(pVertexData, pAiMesh->mVertices,
      allocSize);

    kore::MeshAttributeArray att;
    att.name = "v_position";
    att.numValues = pAiMesh->mNumVertices * 3;
    att.numComponents = 3;
    att.type = GL_FLOAT_VEC3;
    att.componentType = GL_FLOAT;
    att.byteSize = kore::DatatypeUtil::getSizeFromGLdatatype(att.type);
    att.data = pVertexData;
    pMesh->_attributes.push_back(att);
}

void kore::SceneLoader::
  loadVertexNormals(const aiMesh* pAiMesh,
  kore::MeshPtr& pMesh ) {
    unsigned int allocSize = pAiMesh->mNumVertices * 3 * 4;
    void* pVertexData = malloc(allocSize);
    memcpy(pVertexData, pAiMesh->mNormals,
      allocSize);

    kore::MeshAttributeArray att;
    att.name = "v_normal";
    att.numValues = pAiMesh->mNumVertices * 3;
    att.numComponents = 3;
    att.type = GL_FLOAT_VEC3;
    att.componentType = GL_FLOAT;
    att.byteSize = kore::DatatypeUtil::getSizeFromGLdatatype(att.type);
    att.data = pVertexData;
    pMesh->_attributes.push_back(att);
}

void kore::SceneLoader::
  loadVertexTangents(const aiMesh* pAiMesh,
  kore::MeshPtr& pMesh) {
    unsigned int allocSize = pAiMesh->mNumVertices * 3 * 4;
    void* pVertexData = malloc(allocSize);
    memcpy(pVertexData, pAiMesh->mTangents,
      allocSize);

    kore::MeshAttributeArray att;
    att.name = "v_tangent";
    att.numValues = pAiMesh->mNumVertices * 3;
    att.numComponents = 3;
    att.type = GL_FLOAT_VEC3;
    att.componentType = GL_FLOAT;
    att.byteSize = kore::DatatypeUtil::getSizeFromGLdatatype(att.type);
    att.data = pVertexData;
    pMesh->_attributes.push_back(att);
}

void kore::SceneLoader::
  loadFaceIndices(const aiMesh* pAiMesh,
  kore::MeshPtr& pMesh ) {
    for (unsigned int iFace = 0; iFace < pAiMesh->mNumFaces; ++iFace) {
      aiFace& rAiFace = pAiMesh->mFaces[iFace];
      for (unsigned int iIndex = 0; iIndex < rAiFace.mNumIndices; ++iIndex) {
        pMesh->_indices.push_back(rAiFace.mIndices[iIndex]);
      }
    }
}

void kore::SceneLoader::
  loadVertexColors(const aiMesh* pAiMesh,
  kore::MeshPtr& pMesh,
  unsigned int iColorSet) {
    unsigned int allocSize =
      pAiMesh->mNumVertices * 4 * pAiMesh->GetNumColorChannels();
    void* pVertexData = malloc(allocSize);
    memcpy(pVertexData, pAiMesh->mColors[iColorSet], allocSize);

    kore::MeshAttributeArray att;
    char szNameBuf[20];
    sprintf(szNameBuf, "v_color%i", iColorSet);
    att.name = std::string(&szNameBuf[0]);
    att.numValues = pAiMesh->mNumVertices * pAiMesh->GetNumColorChannels();
    att.numComponents = pAiMesh->GetNumColorChannels();

    if (pAiMesh->GetNumColorChannels() == 2) {
      att.type = GL_FLOAT_VEC2;
    } else if (pAiMesh->GetNumColorChannels() == 3) {
      att.type = GL_FLOAT_VEC3;
    } else if (pAiMesh->GetNumColorChannels() == 4) {
      att.type = GL_FLOAT_VEC4;
    } else {
      Log::getInstance()->write("[WARNING] Mesh %s has an"
        "unsupported number of color channels: %i",
        pMesh->getName().c_str());
      free(pVertexData);
      return;
    }

    att.componentType = GL_FLOAT;
    att.byteSize = kore::DatatypeUtil::getSizeFromGLdatatype(att.type);
    att.data = pVertexData;
    pMesh->_attributes.push_back(att);
}

void kore::SceneLoader::
  loadVertexTextureCoords(const aiMesh* pAiMesh,
  kore::MeshPtr& pMesh,
  unsigned int iUVset) {
    unsigned int allocSize =
      pAiMesh->mNumVertices * 4 * pAiMesh->GetNumUVChannels();
    void* pVertexData = malloc(allocSize);
    memcpy(pVertexData, pAiMesh->mTextureCoords[iUVset], allocSize);

    kore::MeshAttributeArray att;
    char szNameBuf[20];
    sprintf(szNameBuf, "v_uv%i", iUVset);
    att.name = std::string(&szNameBuf[0]);
    att.numValues = pAiMesh->mNumVertices * pAiMesh->GetNumUVChannels();
    att.numComponents = pAiMesh->GetNumUVChannels();

    if (pAiMesh->GetNumUVChannels() == 2) {
      att.type = GL_FLOAT_VEC2;
    } else if (pAiMesh->GetNumUVChannels() == 3) {
      att.type = GL_FLOAT_VEC3;
    } else {
      Log::getInstance()->write("[WARNING] Mesh %s has an unsupported"
        "number of UV channels: %i",
        pMesh->getName().c_str());
      free(pVertexData);
      return;
    }

    att.componentType = GL_FLOAT;
    att.byteSize = kore::DatatypeUtil::getSizeFromGLdatatype(att.type);
    att.data = pVertexData;
    pMesh->_attributes.push_back(att);
}

glm::mat4 kore::SceneLoader::glmMatFromAiMat(const aiMatrix4x4& aiMat) {
  // Note: ai-matrix is row-major, but glm::mat4 is column-major
  return glm::mat4(aiMat.a1, aiMat.b1, aiMat.c1, aiMat.d1,
    aiMat.a2, aiMat.b2, aiMat.c2, aiMat.d2,
    aiMat.a3, aiMat.b3, aiMat.c3, aiMat.d3,
    aiMat.a4, aiMat.b4, aiMat.c4, aiMat.d4);
}