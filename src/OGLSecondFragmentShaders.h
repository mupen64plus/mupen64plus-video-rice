/*
Copyright (C) 2014 Dorian Fevrier

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef _OGL_SECOND_FRAGMENT_SHADER_H_
#define _OGL_SECOND_FRAGMENT_SHADER_H_

/*###############################################"

class COGLSecondFragmentProgramCombiner : public CColorCombiner
{
public:
    bool Initialize(void);

protected:
    friend class OGLDeviceBuilder;

    void DisableCombiner(void);
    void InitCombinerCycleCopy(void);
    void InitCombinerCycleFill(void);
    void InitCombinerCycle12(void);

    COGLSecondFragmentProgramCombiner(CRender *pRender);
    ~COGLSecondFragmentProgramCombiner();


private:
    virtual int ParseDecodedMux();
    virtual void GenerateProgramStr();
    int FindCompiledMux();
    virtual void GenerateCombinerSetting(int index);
    virtual void GenerateCombinerSettingConstants(int index);

#ifdef DEBUGGER
    void DisplaySimpleMuxString(void);
#endif

};


###############################################"*/

#include <vector>

#include "osal_opengl.h"

#include "OGLCombiner.h"
#include "OGLExtCombiner.h"
#include "GeneralCombiner.h"

#define CC_NULL_PROGRAM      0 // Invalid OpenGL program
#define CC_NULL_SHADER       0 // Invalid OpenGL shader
#define CC_INACTIVE_UNIFORM -1 // Invalid program uniform

typedef struct {
    uint32 combineMode1;
    uint32 combineMode2;
    unsigned int cycle_type;    // 1/2/fill/copy
    unsigned int key_enabled:1;   // Chroma key
    uint16       blender;
    unsigned int alpha_compare; // None/Threshold/Dither
    unsigned int aa_en:1;
    unsigned int z_cmp:1;
    unsigned int z_upd:1;
    unsigned int alpha_cvg_sel:1;
    unsigned int cvg_x_alpha:1;
    unsigned int fog_enabled:1;
    unsigned int fog_in_blender:1;
    //unsigned int clr_on_cvg;
    //unsigned int cvg_dst;
    GLuint program;
    // Progam uniform locations
    GLint fogMaxMinLoc;
    GLint blendColorLoc;
    GLint primColorLoc;
    GLint envColorLoc;
    GLint chromaKeyCenterLoc;
    GLint chromaKeyScaleLoc;
    GLint chromaKeyWidthLoc;
    GLint lodFracLoc;
    GLint primLodFracLoc;
    GLint k5Loc;
    GLint k4Loc;
    GLint tex0Loc;
    GLint tex1Loc;
    GLint fogColorLoc;
} OGLSecondShaderCombinerSaveType;

class COGLSecondFragmentProgramCombiner : public COGLColorCombiner4
{
public:
    bool Initialize(void);

protected:
    friend class OGLDeviceBuilder;

    void DisableCombiner(void);
    void InitCombinerCycleCopy(void);
    void InitCombinerCycleFill(void);
    void InitCombinerCycle12(void);

    COGLSecondFragmentProgramCombiner(CRender *pRender);
    ~COGLSecondFragmentProgramCombiner();

    std::vector<OGLSecondShaderCombinerSaveType> m_vGeneratedPrograms;
    int m_currentShaderId;
    GLuint m_currentProgram;

private:
    virtual int ParseDecodedMux();
    void genFragmentBlenderStr( char *newFrgStr );
    virtual GLuint GenerateCycle12Program();
    virtual GLuint GenerateCopyProgram();
    virtual void GenerateProgramStr();
    //int FindCompiledMux();
    virtual void GenerateCombinerSetting();
    virtual void GenerateCombinerSettingConstants( int shaderId );
    void StoreUniformLocations( OGLSecondShaderCombinerSaveType &saveType );
    int FindCompiledShaderId();
    
    GLuint m_vtxShader;   // Generate vertex shader once as it never change
    GLuint m_fillProgram; // Generate fill program once as it never change
    GLint  m_fillColorLoc;

#ifdef DEBUGGER
    void DisplaySimpleMuxString(void);
#endif

};

#endif

