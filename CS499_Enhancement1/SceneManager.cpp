///////////////////////////////////////////////////////////////////////////////
// SceneManager.cpp
// ===============
// Manage the loading and rendering of 3D scenes
//
// Author: Karina Washington (updated for CS-499 – Nov 2025)
// Note: This version intentionally uses ONLY the Plane mesh so you don’t
//       depend on any other ShapeMeshes loaders (cube/cylinder/etc.).
//       All objects (desk, monitor, keyboard, mouse, notebook, coaster)
//       are composed by scaling/translating planes.
//       This keeps your build stable with the ShapeMeshes you already have.
//
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>
#include <iostream>

// === Shader uniform names expected by the provided ShaderManager ===
namespace
{
    const char* g_ModelName        = "model";
    const char* g_ColorValueName   = "objectColor";
    const char* g_TextureValueName = "objectTexture";
    const char* g_UseTextureName   = "bUseTexture";
    // Keep g_UseLightingName declared in case your fragment shader uses it.
    const char* g_UseLightingName  = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 ***********************************************************/
SceneManager::SceneManager(ShaderManager* pShaderManager)
{
    m_pShaderManager = pShaderManager;
    m_basicMeshes    = new ShapeMeshes();

    m_loadedTextures = 0;
    // You can push materials here later if you want to use SetShaderMaterial(...)
}

/***********************************************************
 *  ~SceneManager()
 ***********************************************************/
SceneManager::~SceneManager()
{
    m_pShaderManager = NULL;
    if (m_basicMeshes)
    {
        delete m_basicMeshes;
        m_basicMeshes = NULL;
    }
}

/***********************************************************
 *  CreateGLTexture()
 *  (Not used in this minimal, color-only scene, but kept
 *   here so your class remains complete.)
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
    int width = 0, height = 0, colorChannels = 0;
    GLuint textureID = 0;

    stbi_set_flip_vertically_on_load(true);

    unsigned char* image = stbi_load(filename, &width, &height, &colorChannels, 0);
    if (image)
    {
        std::cout << "Loaded image: " << filename
                  << "  w:" << width << "  h:" << height
                  << "  ch:" << colorChannels << std::endl;

        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (colorChannels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8,  width, height, 0, GL_RGB,  GL_UNSIGNED_BYTE, image);
        else if (colorChannels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            std::cout << "Unsupported channel count: " << colorChannels << std::endl;
            stbi_image_free(image);
            glBindTexture(GL_TEXTURE_2D, 0);
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0);

        m_textureIDs[m_loadedTextures].ID  = textureID;
        m_textureIDs[m_loadedTextures].tag = tag;
        m_loadedTextures++;
        return true;
    }

    std::cout << "Could not load image: " << filename << std::endl;
    return false;
}

/***********************************************************
 *  BindGLTextures()
 ***********************************************************/
void SceneManager::BindGLTextures()
{
    for (int i = 0; i < m_loadedTextures; i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
    }
}

/***********************************************************
 *  DestroyGLTextures()
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
    for (int i = 0; i < m_loadedTextures; i++)
    {
        glDeleteTextures(1, &m_textureIDs[i].ID);
    }
    m_loadedTextures = 0;
}

/***********************************************************
 *  FindTextureID()
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
    for (int i = 0; i < m_loadedTextures; ++i)
        if (m_textureIDs[i].tag == tag) return m_textureIDs[i].ID;
    return -1;
}

/***********************************************************
 *  FindTextureSlot()
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
    for (int i = 0; i < m_loadedTextures; ++i)
        if (m_textureIDs[i].tag == tag) return i;
    return -1;
}

/***********************************************************
 *  FindMaterial()
 *  (Stub kept for completeness; not used in this minimal pass)
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
    if (m_objectMaterials.empty()) return false;
    for (auto& m : m_objectMaterials)
    {
        if (m.tag == tag)
        {
            material = m;
            return true;
        }
    }
    return false;
}

/***********************************************************
 *  SetTransformations()
 ***********************************************************/
void SceneManager::SetTransformations(
    glm::vec3 scaleXYZ,
    float XrotationDegrees,
    float YrotationDegrees,
    float ZrotationDegrees,
    glm::vec3 positionXYZ)
{
    glm::mat4 model(1.0f);
    glm::mat4 S = glm::scale(scaleXYZ);
    glm::mat4 Rx = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1,0,0));
    glm::mat4 Ry = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0,1,0));
    glm::mat4 Rz = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0,0,1));
    glm::mat4 T  = glm::translate(positionXYZ);

    model = T * Rx * Ry * Rz * S;

    if (m_pShaderManager)
        m_pShaderManager->setMat4Value(g_ModelName, model);
}

/***********************************************************
 *  SetShaderColor()
 ***********************************************************/
void SceneManager::SetShaderColor(
    float r, float g, float b, float a)
{
    if (m_pShaderManager)
    {
        m_pShaderManager->setIntValue(g_UseTextureName, false);
        m_pShaderManager->setVec4Value(g_ColorValueName, glm::vec4(r,g,b,a));
    }
}

/***********************************************************
 *  SetShaderTexture()
 ***********************************************************/
void SceneManager::SetShaderTexture(std::string textureTag)
{
    if (!m_pShaderManager) return;

    m_pShaderManager->setIntValue(g_UseTextureName, true);
    int slot = FindTextureSlot(textureTag);
    if (slot >= 0)
        m_pShaderManager->setSampler2DValue(g_TextureValueName, slot);
    else
        m_pShaderManager->setIntValue(g_UseTextureName, false); // fallback to color
}

/***********************************************************
 *  SetTextureUVScale()
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
    if (m_pShaderManager)
        m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
}

/***********************************************************
 *  SetShaderMaterial()
 *  (Not used in color-only pass; safe to keep.)
 ***********************************************************/
void SceneManager::SetShaderMaterial(std::string materialTag)
{
    OBJECT_MATERIAL mat{};
    if (FindMaterial(materialTag, mat) && m_pShaderManager)
    {
        m_pShaderManager->setVec3Value("material.ambientColor",  mat.ambientColor);
        m_pShaderManager->setFloatValue("material.ambientStrength", mat.ambientStrength);
        m_pShaderManager->setVec3Value("material.diffuseColor",  mat.diffuseColor);
        m_pShaderManager->setVec3Value("material.specularColor", mat.specularColor);
        m_pShaderManager->setFloatValue("material.shininess",    mat.shininess);
    }
}

/***********************************************************
 *  PrepareScene()
 *  Loads only the Plane mesh (single dependency).
 ***********************************************************/
void SceneManager::PrepareScene()
{
    // One mesh, many draws: keep memory simple and predictable.
    m_basicMeshes->LoadPlaneMesh();

    // If you later add texture files, you can register them here with tags:
    // CreateGLTexture("../../Utilities/textures/wood.png", "WOOD");
    // CreateGLTexture("../../Utilities/textures/metal.png", "METAL");
    // BindGLTextures();
}

/***********************************************************
 *  RenderScene()
 *  Draw multiple planes with transforms to build the scene.
 ***********************************************************/
void SceneManager::RenderScene()
{
    // Common vars
    glm::vec3 scaleXYZ;
    float rx = 0.0f, ry = 0.0f, rz = 0.0f;
    glm::vec3 posXYZ;

    // -------------------------------------------------------
    // 1) DESK TOP (large plane)
    // -------------------------------------------------------
    scaleXYZ = glm::vec3(20.0f, 1.0f, 10.0f);   // wide and deep, thin
    rx = 0.0f; ry = 0.0f; rz = 0.0f;
    posXYZ = glm::vec3(0.0f, 0.0f, 0.0f);       // centered
    SetTransformations(scaleXYZ, rx, ry, rz, posXYZ);
    SetShaderColor(0.55f, 0.32f, 0.18f, 1.0f);  // brownish
    m_basicMeshes->DrawPlaneMesh();

    // -------------------------------------------------------
    // 2) MONITOR SCREEN (upright plane)
    // -------------------------------------------------------
    scaleXYZ = glm::vec3(6.0f, 3.5f, 0.2f);     // wide rectangle
    rx = 0.0f; ry = 0.0f; rz = 0.0f;
    posXYZ = glm::vec3(0.0f, 3.0f, -3.0f);      // lifted off desk, toward back
    SetTransformations(scaleXYZ, rx, ry, rz, posXYZ);
    SetShaderColor(0.05f, 0.05f, 0.07f, 1.0f);  // dark bezel
    m_basicMeshes->DrawPlaneMesh();

    // Screen panel (slightly in front, lighter color to suggest display)
    scaleXYZ = glm::vec3(5.6f, 3.1f, 0.2f);
    posXYZ   = glm::vec3(0.0f, 3.0f, -2.98f);
    SetTransformations(scaleXYZ, rx, ry, rz, posXYZ);
    SetShaderColor(0.12f, 0.16f, 0.22f, 1.0f);  // dark blue-gray “screen”
    m_basicMeshes->DrawPlaneMesh();

    // -------------------------------------------------------
    // 3) KEYBOARD (flat plane)
    // -------------------------------------------------------
    scaleXYZ = glm::vec3(6.0f, 0.2f, 2.0f);
    rx = -2.0f; ry = 0.0f; rz = 0.0f;           // slight tilt
    posXYZ = glm::vec3(0.0f, 0.6f, -0.5f);
    SetTransformations(scaleXYZ, rx, ry, rz, posXYZ);
    SetShaderColor(0.18f, 0.18f, 0.20f, 1.0f);  // dark gray
    m_basicMeshes->DrawPlaneMesh();

    // -------------------------------------------------------
    // 4) MOUSE (small flat plane)
    // -------------------------------------------------------
    scaleXYZ = glm::vec3(1.0f, 0.2f, 1.5f);
    rx = -2.0f; ry = 0.0f; rz = 0.0f;
    posXYZ = glm::vec3(3.8f, 0.7f, 0.3f);
    SetTransformations(scaleXYZ, rx, ry, rz, posXYZ);
    SetShaderColor(0.22f, 0.22f, 0.24f, 1.0f);
    m_basicMeshes->DrawPlaneMesh();

    // -------------------------------------------------------
    // 5) NOTEBOOK (thin plane)
    // -------------------------------------------------------
    scaleXYZ = glm::vec3(3.0f, 0.2f, 2.2f);
    rx = -2.0f; ry = 12.0f; rz = 0.0f;          // slight open/angle
    posXYZ = glm::vec3(-5.0f, 0.55f, 1.5f);
    SetTransformations(scaleXYZ, rx, ry, rz, posXYZ);
    SetShaderColor(0.85f, 0.85f, 0.88f, 1.0f);  // light gray “paper”
    m_basicMeshes->DrawPlaneMesh();

    // -------------------------------------------------------
    // 6) COASTER (small plane)
    // -------------------------------------------------------
    scaleXYZ = glm::vec3(1.2f, 0.2f, 1.2f);
    rx = 0.0f; ry = 0.0f; rz = 0.0f;
    posXYZ = glm::vec3(2.2f, 0.55f, 2.6f);
    SetTransformations(scaleXYZ, rx, ry, rz, posXYZ);
    SetShaderColor(0.60f, 0.40f, 0.20f, 1.0f);  // wood coaster tone
    m_basicMeshes->DrawPlaneMesh();

    // -------------------------------------------------------
    // 7) MUG (placeholder plane footprint)
    //    If later you decide to use a cylinder mesh, you can replace this.
    // -------------------------------------------------------
    scaleXYZ = glm::vec3(1.0f, 0.2f, 1.0f);
    rx = 0.0f; ry = 0.0f; rz = 0.0f;
    posXYZ = glm::vec3(2.2f, 0.75f, 2.6f);      // slightly above coaster
    SetTransformations(scaleXYZ, rx, ry, rz, posXYZ);
    SetShaderColor(0.9f, 0.9f, 0.95f, 1.0f);    // white mug placeholder
    m_basicMeshes->DrawPlaneMesh();
}