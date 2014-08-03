/*
Copyright (C) 2003 Rice1964

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

#include "OGLSecondFragmentShaders.h"
#include "OGLExtensions.h"
#include "OGLDebug.h"
#include "OGLRender.h"
#include "OGLGraphicsContext.h"
#include "OGLTexture.h"
#include <assert.h>

char newFrgStr[4092]; // The main buffer for store fragment shader string

const char *vertexShaderStr =
"#version " GLSL_VERSION                                   "\n"\
"uniform vec2 uFogMinMax;                          \n"\
"                                                           \n"\
"attribute vec4 inPosition;                          \n"\
"attribute vec2 inTexCoord0;                          \n"\
"attribute vec2 inTexCoord1;                          \n"\
"attribute float inFog;                          \n"\
"attribute vec4 inShadeColor;                             \n"\
"                                                           \n"\
"varying vec2 vertexTexCoord0;                             \n"\
"varying vec2 vertexTexCoord1;                             \n"\
"varying float vertexFog;                             \n"\
"varying vec4 vertexShadeColor;                             \n"\
"                                                           \n"\
"void main()                                                \n"\
"{                                                          \n"\
"gl_Position = inPosition;                                   \n"\
"vertexTexCoord0 = inTexCoord0;                                      \n"\
"vertexTexCoord1 = inTexCoord1;                                      \n"\

"vertexFog = clamp( (uFogMinMax[1] - inFog) / (uFogMinMax[1] - uFogMinMax[0]), 0.0, 1.0); \n"\
"vertexShadeColor = inShadeColor;                                      \n"\
"}                                                          \n"\
"                                                           \n";

const char *fragmentShaderHeader =
"#version " GLSL_VERSION                                   "\n"\
"uniform vec4 uBlendColor;                          \n"\
"uniform vec4 uPrimColor;                          \n"\
"uniform vec4 uEnvColor;                          \n"\
"uniform vec3 uChromaKeyCenter;                   \n"\
"uniform vec3 uChromaKeyScale;                    \n"\
"uniform vec3 uChromaKeyWidth;                    \n"\
"uniform float uLodFrac;                          \n"\
"uniform float uPrimLodFrac;                          \n"\
"uniform float uK5;                          \n"\
"uniform float uK4;                          \n"\
"uniform sampler2D uTex0;                          \n"\
"uniform sampler2D uTex1;                          \n"\
"uniform vec4 uFogColor;                          \n"\
"                                                           \n"\
"varying vec2 vertexTexCoord0;                          \n"\
"varying vec2 vertexTexCoord1;                          \n"\
"varying float vertexFog;                          \n"\
"varying vec4 vertexShadeColor;                          \n"\
"                                                           \n"\
"                                                           \n"\
"void main()                                                \n"\
"{                                                          \n"\
"vec4 outColor;                                             \n";

//Fragment shader for InitCycleCopy
const char *fragmentCopyHeader =
"#version " GLSL_VERSION "\n"\
"uniform vec4 uBlendColor;                          \n"\
"uniform sampler2D uTex0;                                   \n"\
"varying vec2 vertexTexCoord0;                              \n"\
"void main()                                                \n"\
"{                                                          \n"\
"vec4 outColor = texture2D(uTex0,vertexTexCoord0);           \n";

//Fragment shader for InitCycleFill (the only self contain fragment shader)
const char *fragmentFill =
"#version " GLSL_VERSION "\n"\
"uniform vec4 uFillColor;                                   \n"
"void main()                                                \n"
"{                                                          \n"
"gl_FragColor = uFillColor;                                 \n"
"}";

const char *fragmentShaderFooter =
"    gl_FragColor = outColor;                               \n"\
"}                                                          \n"\
"                                                           \n";

GLuint createShader( GLenum shaderType, const char* shaderSrc ) {

    GLuint shader = glCreateShader( shaderType ); // GL_VERTEX_SHADER, GL_FRAGMENT_SHADER

    glShaderSource(shader, 1, &shaderSrc, NULL);
    glCompileShader(shader);
    
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint infoLogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
        
        GLchar strInfoLog[ infoLogLength ];
        glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);
        
        const char *strShaderType = NULL;
        switch(shader)
        {
        case GL_VERTEX_SHADER: strShaderType = "vertex"; break;
        //case GL_GEOMETRY_SHADER: strShaderType = "geometry"; break;
        case GL_FRAGMENT_SHADER: strShaderType = "fragment"; break;
        }
        
        printf( "Compile failure in %s shader:\n%s\n", strShaderType, strInfoLog);
        printf( "GLSL code:\n%s\n", shaderSrc);
    }
    
    return shader;
};

GLuint createProgram(const GLuint vShader, GLuint fShader)
{
    GLuint program = glCreateProgram();
    
    glAttachShader(program, vShader);
    glAttachShader(program, fShader);
    
    glBindAttribLocation(program,VS_POSITION,"inPosition");
    glBindAttribLocation(program,VS_TEXCOORD0,"inTexCoord0");
    glBindAttribLocation(program,VS_TEXCOORD1,"inTexCoord1");
    glBindAttribLocation(program,VS_FOG,"inFog");
    glBindAttribLocation(program,VS_COLOR,"inShadeColor");
    
    glLinkProgram(program);
    
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint infoLogLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
        
        GLchar strInfoLog[ infoLogLength ];
        glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
        printf("Linker failure: %s\n", strInfoLog);
    }
    
    glDetachShader(program, vShader);
    glDetachShader(program, fShader);

    return program;
}

COGLSecondFragmentProgramCombiner::COGLSecondFragmentProgramCombiner(CRender *pRender)
: COGLColorCombiner(pRender)
{
    delete m_pDecodedMux;
    m_pDecodedMux = new DecodedMux;
    m_currentProgram = CC_NULL_PROGRAM;
    
    m_vtxShader = createShader( GL_VERTEX_SHADER, vertexShaderStr );
    
    // Generate Fill program
    GLuint frgShaderFill = createShader( GL_FRAGMENT_SHADER, fragmentFill );
    m_fillProgram  = createProgram(m_vtxShader, frgShaderFill);
    m_fillColorLoc = glGetUniformLocation(m_fillProgram,"uFillColor");
    OPENGL_CHECK_ERRORS
    glDeleteShader( frgShaderFill );
    OPENGL_CHECK_ERRORS
}
COGLSecondFragmentProgramCombiner::~COGLSecondFragmentProgramCombiner()
{
    glDeleteShader(m_vtxShader); OPENGL_CHECK_ERRORS
    
    if ( glIsProgram( m_currentProgram ) != GL_TRUE ){
        glDeleteProgram( m_currentProgram );
    }
}

bool COGLSecondFragmentProgramCombiner::Initialize(void)
{
    return true;
}



void COGLSecondFragmentProgramCombiner::DisableCombiner(void)
{
    glUseProgram(0);
    OPENGL_CHECK_ERRORS
}

void COGLSecondFragmentProgramCombiner::InitCombinerCycleCopy(void)
{
    // Look if we already have a compiled program for the current state.
    int shaderId = FindCompiledShaderId();
    
    if( shaderId == -1 ) {
        shaderId = GenerateCopyProgram();
    }
    
    GLuint program = m_vGeneratedPrograms[shaderId].program;
    if( program != m_currentProgram ) {
        glUseProgram( program );
        OPENGL_CHECK_ERRORS
        m_currentProgram = program;
    }
    
    m_pOGLRender->DisableMultiTexture();
    m_pOGLRender->EnableTexUnit(0,TRUE);
    
    GenerateCombinerSetting();
    GenerateCombinerSettingConstants( shaderId );

    glEnableVertexAttribArray(VS_POSITION);
    OPENGL_CHECK_ERRORS
    glEnableVertexAttribArray(VS_TEXCOORD0);
    OPENGL_CHECK_ERRORS
    glDisableVertexAttribArray(VS_COLOR);
    OPENGL_CHECK_ERRORS
    glDisableVertexAttribArray(VS_TEXCOORD1);
    OPENGL_CHECK_ERRORS
    glDisableVertexAttribArray(VS_FOG);
    OPENGL_CHECK_ERRORS
    COGLTexture* pTexture = g_textures[gRSP.curTile].m_pCOGLTexture;
    if( pTexture )
    {
        m_pOGLRender->BindTexture(pTexture->m_dwTextureName, 0);
        m_pOGLRender->SetTexelRepeatFlags(gRSP.curTile);
    }
}

void COGLSecondFragmentProgramCombiner::InitCombinerCycleFill(void)
{
    // Fill shader is always the same, don't waste time with a lookup.
    if( m_currentProgram != m_fillProgram ) {
        glUseProgram( m_fillProgram );
        OPENGL_CHECK_ERRORS
        m_currentProgram = m_fillProgram;
    }
    glUniform4f(m_fillColorLoc,((gRDP.fillColor>>16)&0xFF)/255.0f,((gRDP.fillColor>>8)&0xFF)/255.0f,((gRDP.fillColor)&0xFF)/255.0f,((gRDP.fillColor>>24)&0xFF)/255.0f);
    OPENGL_CHECK_ERRORS
}

// Generate the Blender (BL) part of the fragment shader 
void COGLSecondFragmentProgramCombiner::genFragmentBlenderStr( char *newFrgStr )
{
    ////////////////////////////////////////////////////////////////////////////
    // BL (Blender). Equation: (A*P+B*M)/(A+B)
    ////////////////////////////////////////////////////////////////////////////
    
    /*uint16 tmpblender = gRDP.otherMode.blender;
    //RDP_BlenderSetting &bl = *(RDP_BlenderSetting*)(&(blender));
    RDP_BlenderSetting* blender = (RDP_BlenderSetting*)(&tmpblender);
    
    switch( blender->c1_m1a ) // A cycle 1
    {
        case 0 : // CC output alpha
            strcat(newFrgStr, "float BL_A = outColor.a;\n");
            break;
        case 1 : // fog alpha (register)
            strcat(newFrgStr, "float BL_A = vertexFog;\n");
            break;
        case 2 : // shade alpha
            strcat(newFrgStr, "float BL_A = vertexShadeColor.a;\n");
            break;
        case 3 : // 0.0
            strcat(newFrgStr, "float BL_A = 0.0;\n");
            break;
    }
    switch( blender->c1_m1b ) // P cycle 1 (Same as M)
    {
        case 0 : // pixel rgb
            strcat(newFrgStr, "vec3 BL_P = outColor.rgb;\n");
            break;
        case 1 :
            // TODO: 1 memory RGB (what is that?)
            //strcat(newFrgStr, "vec3 BL_P = vec3(1.0, 0.0, 1.0);\n"); //purple...
            strcat(newFrgStr, "vec3 BL_P = uPrimColor.rgb;\n");
            break;
        case 2 : // blend rgb (register)
            strcat(newFrgStr, "vec3 BL_P = uBlendColor.rgb;\n");
            break;
        case 3 : // fog rgb (register)
            //strcat(newFrgStr, "vec3 BL_P = uFogColor.rgb;\n");
            strcat(newFrgStr, "vec3 BL_P = vec3(1.0, 0.0, 1.0);\n"); //purple...
            break;
    }
    switch( blender->c1_m2a ) // B cycle 1
    {
        case 0 : // 1.0 - ‘a mux’ output
            strcat(newFrgStr, "float BL_B = 1.0 - BL_A;\n");
            break;
        case 1 : // memory alpha
            // TODO: memory alpha (what is that?)
            strcat(newFrgStr, "float BL_B = 1.0;\n");
            break;
        case 2 : // 1.0
            strcat(newFrgStr, "float BL_B = 1.0;\n");
            break;
        case 3 : // 0.0
            strcat(newFrgStr, "float BL_B = 0.0;\n");
            break;
    }
    switch( blender->c1_m2b ) // M cycle 1 (Same as P)
    {
        case 0 : // pixel rgb
            strcat(newFrgStr, "vec3 BL_M = outColor.rgb;\n");
            break;
        case 1 :
            // TODO: 1 memory RGB (what is that?)
            //strcat(newFrgStr, "vec3 BL_M = vec3(1.0, 0.0, 1.0);\n"); //purple...
            strcat(newFrgStr, "vec3 BL_M = uPrimColor.rgb;\n"); 
            break;
        case 2 : // blend rgb (register)
            strcat(newFrgStr, "vec3 BL_M = uBlendColor.rgb;\n");
            break;
        case 3 : // fog rgb (register)
            //strcat(newFrgStr, "vec3 BL_M = uFogColor.rgb;\n");
            strcat(newFrgStr, "vec3 BL_M = vec3(1.0, 0.0, 1.0);\n"); //purple...
            break;
    }
    strcat(newFrgStr, "outColor = ( vec4(BL_P.rgb, BL_A) + vec4(BL_M.rgb, BL_B) ) / vec4( BL_A + BL_B );\n");*/
    
    // When aa_en and alpha_cvg_sel are enabled, N64 do a nice and smoother clamp (aka coverage is smooth).
    // This try to simulate the "smooth" clamp increasing the value to avoid pixelized clamp
    /*if( gRDP.otherMode.aa_en )
    {
        strcat(newFrgStr, "float clamp_value = 0.5;\n");
    } else {
        strcat(newFrgStr, "float clamp_value = 0.004;\n"); // default value 1/255 = 0.004
    }*/
    /*    // As we don't have coverage in GLSL, we consider coverage = alpha
        if( gRDP.otherMode.alpha_cvg_sel ) // Use coverage bits for alpha calculation
        {
            if( gRDP.otherMode.cvg_x_alpha )
            {
                // texture cutout mode (Mario Kart 64 kart sprites/Mario 64 trees/some 1080SB  face to cam border trees)
                strcat(newFrgStr, "if( outColor.a < clamp_value ) discard;\n");
            }
            else
            {
                //strcat(newFrgStr, "outColor.a = 1.0;\n");
            }
        } else if( gRDP.otherMode.cvg_x_alpha )
        {
            // strict texture cutout mode (Mario Kart 64 items sprites /1080SB border trees)
            strcat(newFrgStr, "if( outColor.a < 0.004 ) discard;\n");
        }*/
    //}
    
    strcat(newFrgStr, "float coverage = 1.0;\n");
    if( gRDP.otherMode.cvg_x_alpha )
    {
        /*strcat(newFrgStr, "if( outColor.a > 0.004 ) {\n");
        strcat(newFrgStr, "    coverage = 1.0;\n");
        strcat(newFrgStr, "} else {\n");
        strcat(newFrgStr, "    coverage = 0.0;\n");
        strcat(newFrgStr, "}\n");*/
        strcat(newFrgStr, "coverage = coverage * outColor.a;\n");
    }
    if( gRDP.otherMode.alpha_cvg_sel ) // Use coverage bits for alpha calculation
    {
        // texture cutof (MK64 kart sprites/Mario 64 trees/1080SB "face to cam" border trees)
        strcat(newFrgStr, "coverage = step( 0.5, coverage );\n");
        strcat(newFrgStr, "outColor.a = coverage;\n");
    }
    /*if( gRDP.otherMode.clr_on_cvg ) // SSB64 first ground sprites
    {
        strcat(newFrgStr, "if( coverage < 0.004 ) discard;\n");
    }*/
    
    /*switch( gRDP.otherMode.cvg_dst ) {
        case CVG_DST_CLAMP :
            strcat(newFrgStr, "if( coverage < 0.99 ) coverage = 0.0 ;\n");
            break;
        case CVG_DST_WRAP :
            strcat(newFrgStr, "if( coverage > 0.004 ) coverage = 1.0 ;\n");
            break;
        case CVG_DST_FULL :
            strcat(newFrgStr, "coverage = 1.0 ;\n");
            break;
        case CVG_DST_SAVE :
            strcat(newFrgStr, "coverage = 0.0 ;\n");
            break;
        default :
            break;
    }*/
    
    strcat(newFrgStr, "if( coverage < 0.1 ) discard;\n");

    if( gRDP.otherMode.z_cmp )
    {

    }

    if( gRDP.otherMode.z_upd )
    {
        //strcat(newFrgStr, "gl_FragDepth = gl_FragCoord.z/gl_FragCoord.w;\n");
    }

    switch( gRDP.otherMode.alpha_compare )
    {
        case RDP_ALPHA_COMPARE_THRESHOLD :
            // keep value if outColor.a >= uBlendColor.a
            //strcat(newFrgStr, "if( abs( uBlendColor.a - outColor.a ) < 0.00000001 ) discard;\n"); // Top Gear Rally use this mode for opacity
            strcat(newFrgStr, "if( outColor.a < uBlendColor.a ) discard;\n");
            break;
        case RDP_ALPHA_COMPARE_DITHER :
            strcat(newFrgStr, "if( outColor.a < fract(sin(dot(gl_FragCoord.xy, vec2(12.9898, 78.233)))* 43758.5453) ) discard;\n");
            break;
        case RDP_ALPHA_COMPARE_NONE :
        default :
            break;
    }
    
    if( gRDP.bFogEnableInBlender && gRSP.bFogEnabled ) {
        strcat(newFrgStr, "outColor.rgb = mix(uFogColor.rgb, outColor.rgb, vertexFog);\n");
        //strcat(newFrgStr, "outColor.rgb = vec3(vertexFog,vertexFog,vertexFog);\n");
    }
}

// Generate a program for the current combiner state
// The main part of the function add one line for each part of the combiner.
// A, B, C, D (colors) and a, b, c, d (alpha). So if you modify the part of one of them
// don't forget to modify the part for the other (A and a, B and b, etc...) 
GLuint COGLSecondFragmentProgramCombiner::GenerateCycle12Program()
{
    newFrgStr[0] = 0;
    
    strcat(newFrgStr, fragmentShaderHeader);
    
    ////////////////////////////////////////////////////////////////////////////
    // Colors (rgb) Cycle 1
    ////////////////////////////////////////////////////////////////////////////
    
    // A0 (Cycle 1)
    switch( m_sources[0]  )
    {
        case CCMUX_COMBINED : // 0 (this commented numbers are related to the Color Combiner schema)
            printf("CCMUX_COMBINED for AColor. This should never happen in cycle 1.\n");
            //strcat(newFrgStr, "vec3 AColor = vec3(1.0, 1.0, 1.0);\n"); // set "something".
            break;
        case CCMUX_TEXEL0 : // 1
            strcat(newFrgStr, "vec3 AColor = texture2D(uTex0,vertexTexCoord0).rgb;\n");
            break;
        case CCMUX_TEXEL1 : // 2
            strcat(newFrgStr, "vec3 AColor = texture2D(uTex1,vertexTexCoord1).rgb;\n");
            break;
        case CCMUX_PRIMITIVE : // 3
            strcat(newFrgStr, "vec3 AColor = uPrimColor.rgb;\n");
            break;
        case CCMUX_SHADE : // 4
            strcat(newFrgStr, "vec3 AColor = vertexShadeColor.rgb;\n");
            break;
        case CCMUX_ENVIRONMENT : // 5
            strcat(newFrgStr, "vec3 AColor = uEnvColor.rgb;\n");
            break;
        case CCMUX_1 : // 6
            strcat(newFrgStr, "vec3 AColor = vec3(1.0);\n");
            break;
        case CCMUX_NOISE : // 7
            strcat(newFrgStr, "vec3 AColor = noise3(0.0);\n");
            break;
        case CCMUX_0 : // 8
        default: // everything > CCMUX_0 (8) is 0
            strcat(newFrgStr, "vec3 AColor = vec3(0.0);\n");
            break;
    }
    
    // B0 (Cycle 1)
    switch( m_sources[1]  )
    {
        case CCMUX_COMBINED : // 0
            printf("CCMUX_COMBINED for BColor. This should never happen in cycle 1.\n");
            //strcat(newFrgStr, "vec3 BColor = vec3(1.0, 1.0, 1.0);\n"); // set "something".
            break;
        case CCMUX_TEXEL0 : // 1
            strcat(newFrgStr, "vec3 BColor = texture2D(uTex0,vertexTexCoord0).rgb;\n");
            break;
        case CCMUX_TEXEL1 :// 2
            strcat(newFrgStr, "vec3 BColor = texture2D(uTex1,vertexTexCoord1).rgb;\n");
            break;
        case CCMUX_PRIMITIVE : // 3
            strcat(newFrgStr, "vec3 BColor = uPrimColor.rgb;\n");
            break;
        case CCMUX_SHADE : // 4
            strcat(newFrgStr, "vec3 BColor = vertexShadeColor.rgb;\n");
            break;
        case CCMUX_ENVIRONMENT : // 5
            strcat(newFrgStr, "vec3 BColor = uEnvColor.rgb;\n");
            break;
        case CCMUX_CENTER : // 6
            printf("CCMUX_CENTER for BColor. This should never happen in cycle 1.\n");
            //strcat(newFrgStr, "vec3 BColor = uChromaKeyCenter;\n");
            break;
        case CCMUX_K4 : // 7
            strcat(newFrgStr, "vec3 BColor = vec3(uK4);\n");
            break;
        case CCMUX_0 : // 8
        default: // everything > CCMUX_0 (8) is 0
            strcat(newFrgStr, "vec3 BColor = vec3(0.0);\n");
            break;
    }
    
    // C0 (Cycle 1)
    switch( m_sources[2]  )
    {
        case CCMUX_COMBINED : // 0
            printf("CCMUX_COMBINED for CColor. This should never happen in cycle 1.\n");
            //strcat(newFrgStr, "vec3 CColor = vec3(1.0, 1.0, 1.0);\n"); // set "something".
            break;
        case CCMUX_TEXEL0 : // 1
            strcat(newFrgStr, "vec3 CColor = texture2D(uTex0,vertexTexCoord0).rgb;\n");
            break;
        case CCMUX_TEXEL1 :// 2
            strcat(newFrgStr, "vec3 CColor = texture2D(uTex1,vertexTexCoord1).rgb;\n");
            break;
        case CCMUX_PRIMITIVE : // 3
            strcat(newFrgStr, "vec3 CColor = uPrimColor.rgb;\n");
            break;
        case CCMUX_SHADE : // 4
            strcat(newFrgStr, "vec3 CColor = vertexShadeColor.rgb;\n");
            break;
        case CCMUX_ENVIRONMENT : // 5
            strcat(newFrgStr, "vec3 CColor = uEnvColor.rgb;\n");
            break;
        case CCMUX_SCALE : // 6
            printf("CCMUX_SCALE for CColor. This should never happen in cycle 1.\n");
            //strcat(newFrgStr, "vec3 CColor = uChromaKeyScale;\n");
            break;
        case CCMUX_COMBINED_ALPHA : // 7
            printf("CCMUX_COMBINED_ALPHA for CColor. This should never happen in cycle 1.\n");
            //strcat(newFrgStr, "vec3 CColor = uEnvColor;\n");
            break;
        case CCMUX_TEXEL0_ALPHA : // 8
            strcat(newFrgStr, "vec3 CColor = vec3(texture2D(uTex0,vertexTexCoord0).a);\n");
            break;
        case CCMUX_TEXEL1_ALPHA : // 9
            strcat(newFrgStr, "vec3 CColor = vec3(texture2D(uTex1,vertexTexCoord1).a);\n");
            break;
        case CCMUX_PRIMITIVE_ALPHA : // 10
            strcat(newFrgStr, "vec3 CColor = vec3(uPrimColor.a);\n");
            break;
        case CCMUX_SHADE_ALPHA : // 11
            strcat(newFrgStr, "vec3 CColor = vec3(vertexShadeColor.a);\n");
            break;
        case CCMUX_ENV_ALPHA : // 12
            strcat(newFrgStr, "vec3 CColor = vec3(uEnvColor.a);\n");
            break;
        case CCMUX_LOD_FRACTION : // 13
            strcat(newFrgStr, "vec3 CColor = vec3(uLodFrac);\n"); // Used by Goldeneye64
            break;
        case CCMUX_PRIM_LOD_FRAC : // 14
            strcat(newFrgStr, "vec3 CColor = vec3(uPrimLodFrac);\n"); // Used by Doom64
            break;
        case CCMUX_K5 : // 15
            strcat(newFrgStr, "vec3 CColor = vec3(uK5);\n");
            break;
        case CCMUX_0 : // 16
        default: // everything > CCMUX_0 (16) is 0
            strcat(newFrgStr, "vec3 CColor = vec3(0.0);\n");
            break;
    }
    
    // D0 (Cycle 1)
    switch( m_sources[3]  )
    {
        case CCMUX_COMBINED : // 0
            printf("CCMUX_COMBINED for DColor. This should never happen in cycle 1.\n");
            //strcat(newFrgStr, "vec3 DColor = vec3(1.0, 1.0, 1.0);\n"); // set "something".
            break;
        case CCMUX_TEXEL0 : // 1
            strcat(newFrgStr, "vec3 DColor = texture2D(uTex0,vertexTexCoord0).rgb;\n");
            break;
        case CCMUX_TEXEL1 :// 2
            strcat(newFrgStr, "vec3 DColor = texture2D(uTex1,vertexTexCoord1).rgb;\n");
            break;
        case CCMUX_PRIMITIVE : // 3
            strcat(newFrgStr, "vec3 DColor = uPrimColor.rgb;\n");
            break;
        case CCMUX_SHADE : // 4
            strcat(newFrgStr, "vec3 DColor = vertexShadeColor.rgb;\n");
            break;
        case CCMUX_ENVIRONMENT : // 5
            strcat(newFrgStr, "vec3 DColor = uEnvColor.rgb;\n");
            break;
        case CCMUX_1 : // 6
            strcat(newFrgStr, "vec3 DColor = vec3(1.0);\n");
            break;
        case CCMUX_0 : // 7
        default: // everything > CCMUX_0 (7) is 0
            strcat(newFrgStr, "vec3 DColor = vec3(0.0);\n");
            break;
    }
    
    ////////////////////////////////////////////////////////////////////////////
    // Alphas (float) Cycle 1
    ////////////////////////////////////////////////////////////////////////////
    
    // a0 (Cycle 1) (same than b0 and c0 actually)
    switch( m_sources[4]  )
    {
        case ACMUX_COMBINED : // 0
            printf("ACMUX_COMBINED for AAlpha. This should never happen in cycle 1.\n");
            //strcat(newFrgStr, "float AAlpha = 1.0;\n"); // set "something".
            break;
        case ACMUX_TEXEL0 : // 1
            strcat(newFrgStr, "float AAlpha = texture2D(uTex0,vertexTexCoord0).a;\n");
            break;
        case ACMUX_TEXEL1 : // 2
            strcat(newFrgStr, "float AAlpha = texture2D(uTex1,vertexTexCoord1).a;\n");
            break;
        case ACMUX_PRIMITIVE : // 3
            strcat(newFrgStr, "float AAlpha = uPrimColor.a;\n");
            break;
        case ACMUX_SHADE : // 4
            strcat(newFrgStr, "float AAlpha = vertexShadeColor.a;\n");
            break;
        case ACMUX_ENVIRONMENT : // 5
            strcat(newFrgStr, "float AAlpha = uEnvColor.a;\n");
            break;
        case ACMUX_1 : // 6
            strcat(newFrgStr, "float AAlpha = 1.0;\n");
            break;
        case ACMUX_0 : // 7
        default: // everything > CCMUX_0 (7) is 0
            strcat(newFrgStr, "float AAlpha = 0.0;\n");
            break;
    }
    
    // b0 (Cycle 1) (same than a0 and c0 actually)
    switch( m_sources[5]  )
    {
        case ACMUX_COMBINED : // 0
            printf("ACMUX_COMBINED for BAlpha. This should never happen in cycle 1.\n");
            //strcat(newFrgStr, "float BAlpha = 1.0;\n"); // set "something".
            break;
        case ACMUX_TEXEL0 : // 1
            strcat(newFrgStr, "float BAlpha = texture2D(uTex0,vertexTexCoord0).a;\n");
            break;
        case ACMUX_TEXEL1 : // 2
            strcat(newFrgStr, "float BAlpha = texture2D(uTex1,vertexTexCoord1).a;\n");
            break;
        case ACMUX_PRIMITIVE : // 3
            strcat(newFrgStr, "float BAlpha = uPrimColor.a;\n");
            break;
        case ACMUX_SHADE : // 4
            strcat(newFrgStr, "float BAlpha = vertexShadeColor.a;\n");
            break;
        case ACMUX_ENVIRONMENT : // 5
            strcat(newFrgStr, "float BAlpha = uEnvColor.a;\n");
            break;
        case ACMUX_1 : // 6
            strcat(newFrgStr, "float BAlpha = 1.0;\n");
            break;
        case ACMUX_0 : // 7
        default: // everything > CCMUX_0 (7) is 0
            strcat(newFrgStr, "float BAlpha = 0.0;\n");
            break;
    }
    
    // c0 (Cycle 1) kind of "exotic"
    switch( m_sources[6]  )
    {
        case ACMUX_LOD_FRACTION : // 0
            strcat(newFrgStr, "float CAlpha = uLodFrac;\n");
            break;
        case ACMUX_TEXEL0 : // 1
            strcat(newFrgStr, "float CAlpha = texture2D(uTex0,vertexTexCoord0).a;\n");
            break;
        case ACMUX_TEXEL1 : // 2
            strcat(newFrgStr, "float CAlpha = texture2D(uTex1,vertexTexCoord1).a;\n");
            break;
        case ACMUX_PRIMITIVE : // 3
            strcat(newFrgStr, "float CAlpha = uPrimColor.a;\n");
            break;
        case ACMUX_SHADE : // 4
            strcat(newFrgStr, "float CAlpha = vertexShadeColor.a;\n");
            break;
        case ACMUX_ENVIRONMENT : // 5
            strcat(newFrgStr, "float CAlpha = uEnvColor.a;\n");
            break;
        case ACMUX_PRIM_LOD_FRAC : // 6
            strcat(newFrgStr, "float CAlpha = uPrimLodFrac;\n");
            break;
        case ACMUX_0 : // 7
        default: // everything > CCMUX_0 (7) is 0
            strcat(newFrgStr, "float CAlpha = 0.0;\n");
            break;
    }
    
    // d0 (Cycle 1) (same than a0 and b0 actually)
    switch( m_sources[7]  )
    {
        case ACMUX_COMBINED : // 0
            printf("ACMUX_COMBINED for DAlpha. This should never happen in cycle 1.\n");
            //strcat(newFrgStr, "float DAlpha = 1.0;\n"); // set "something".
            break;
        case ACMUX_TEXEL0 : // 1
            strcat(newFrgStr, "float DAlpha = texture2D(uTex0,vertexTexCoord0).a;\n");
            break;
        case ACMUX_TEXEL1 : // 2
            strcat(newFrgStr, "float DAlpha = texture2D(uTex1,vertexTexCoord1).a;\n");
            break;
        case ACMUX_PRIMITIVE : // 3
            strcat(newFrgStr, "float DAlpha = uPrimColor.a;\n");
            break;
        case ACMUX_SHADE : // 4
            strcat(newFrgStr, "float DAlpha = vertexShadeColor.a;\n");
            break;
        case ACMUX_ENVIRONMENT : // 5
            strcat(newFrgStr, "float DAlpha = uEnvColor.a;\n");
            break;
        case ACMUX_1 : // 6
            strcat(newFrgStr, "float DAlpha = 1.0;\n");
            break;
        case ACMUX_0 : // 7
        default: // everything > CCMUX_0 (7) is 0
            strcat(newFrgStr, "float DAlpha = 0.0;\n");
            break;
    }
    
    strcat(newFrgStr, "vec3 cycle1Color = (AColor - BColor) * CColor + DColor;\n");
    //strcat(newFrgStr, "vec3 cycle1Color = vec3(float(AColor), float(CColor), float(DColor));\n");
    strcat(newFrgStr, "float cycle1Alpha = (AAlpha - BAlpha) * CAlpha + DAlpha;\n");
    //strcat(newFrgStr, "float cycle1Alpha = AAlpha;\n");
    
    //strcat(newFrgStr, "outColor.rgb = vec3(greaterThan(cycle1Color, vec3(1.0)));\n");
    //strcat(newFrgStr, "outColor.rgb = cycle1Color;\n");
    //strcat(newFrgStr, "outColor.a = cycle1Alpha;\n");
    
    switch( gRDP.otherMode.cycle_type )
    {
        case CYCLE_TYPE_1 : // 1 cycle mode? compute the fragment color
            strcat(newFrgStr, "outColor.rgb = cycle1Color;\n");
            strcat(newFrgStr, "outColor.a = cycle1Alpha;\n");
            break;
        case CYCLE_TYPE_2 : { // 2 cycle mode? add another color computation
            // Chroma key
            if( gRDP.otherMode.key_en )
            {
                strcat(newFrgStr, "float resultChromaKeyR = clamp( 0.0, (-abs( (cycle1Color.r - uChromaKeyCenter.r) * uChromaKeyScale.r) + uChromaKeyWidth.r), 1.0);\n");
                strcat(newFrgStr, "float resultChromaKeyG = clamp( 0.0, (-abs( (cycle1Color.g - uChromaKeyCenter.g) * uChromaKeyScale.g) + uChromaKeyWidth.g), 1.0);\n");
                strcat(newFrgStr, "float resultChromaKeyB = clamp( 0.0, (-abs( (cycle1Color.b - uChromaKeyCenter.b) * uChromaKeyScale.b) + uChromaKeyWidth.b), 1.0);\n");
                strcat(newFrgStr, "float resultChromaKeyA = min( resultChromaKeyR, resultChromaKeyG, resultChromaKeyB );\n");
                strcat(newFrgStr, "outColor = vec4( resultChromaKeyR, resultChromaKeyG, resultChromaKeyB, resultChromaKeyA );\n");
            } else { // Color combiner
                ////////////////////////////////////////////////////////////////////////////
                // Colors (rgb) Cycle 2
                ////////////////////////////////////////////////////////////////////////////
                
                // A0 (Cycle 2)
                switch( m_sources[8] )
                {
                    case CCMUX_COMBINED : // 0
                        strcat(newFrgStr, "AColor = cycle1Color;\n");
                        break;
                    case CCMUX_TEXEL0 : // 1
                        strcat(newFrgStr, "AColor = texture2D(uTex0,vertexTexCoord0).rgb;\n");
                        break;
                    case CCMUX_TEXEL1 : // 2
                        strcat(newFrgStr, "AColor = texture2D(uTex1,vertexTexCoord1).rgb;\n");
                        break;
                    case CCMUX_PRIMITIVE : // 3
                        strcat(newFrgStr, "AColor = uPrimColor.rgb;\n");
                        break;
                    case CCMUX_SHADE : // 4
                        strcat(newFrgStr, "AColor = vertexShadeColor.rgb;\n");
                        break;
                    case CCMUX_ENVIRONMENT : // 5
                        strcat(newFrgStr, "AColor = uEnvColor.rgb;\n");
                        break;
                    case CCMUX_1 : // 6
                        strcat(newFrgStr, "AColor = vec3(1.0, 1.0, 1.0);\n");
                        break;
                    case CCMUX_NOISE : // 7
                        strcat(newFrgStr, "AColor = noise3(0.0);\n");
                        break;
                    case CCMUX_0 : // 8
                    default: // everything > CCMUX_0 (8) is 0
                        strcat(newFrgStr, "AColor = vec3(0.0, 0.0, 0.0);\n");
                        break;
                }
                
                // B0 (Cycle 2)
                switch( m_sources[9] )
                {
                    case CCMUX_COMBINED : // 0
                        strcat(newFrgStr, "BColor = cycle1Color;\n");
                        break;
                    case CCMUX_TEXEL0 : // 1
                        strcat(newFrgStr, "BColor = texture2D(uTex0,vertexTexCoord0).rgb;\n");
                        break;
                    case CCMUX_TEXEL1 :// 2
                        strcat(newFrgStr, "BColor = texture2D(uTex1,vertexTexCoord1).rgb;\n");
                        break;
                    case CCMUX_PRIMITIVE : // 3
                        strcat(newFrgStr, "BColor = uPrimColor.rgb;\n");
                        break;
                    case CCMUX_SHADE : // 4
                        strcat(newFrgStr, "BColor = vertexShadeColor.rgb;\n");
                        break;
                    case CCMUX_ENVIRONMENT : // 5
                        strcat(newFrgStr, "BColor = uEnvColor.rgb;\n");
                        break;
                    case CCMUX_CENTER : // 6
                        strcat(newFrgStr, "BColor = uChromaKeyCenter;\n");
                        break;
                    case CCMUX_K4 : // 7
                        strcat(newFrgStr, "BColor = vec3(uK4);\n");
                        break;
                    case CCMUX_0 : // 8
                    default: // everything > CCMUX_0 (8) is 0
                        strcat(newFrgStr, "BColor = vec3(0.0, 0.0, 0.0);\n");
                        break;
                }
                
                // C0 (Cycle 2)
                switch( m_sources[10] )
                {
                    case CCMUX_COMBINED : // 0
                        strcat(newFrgStr, "CColor = cycle1Color;\n");
                        break;
                    case CCMUX_TEXEL0 : // 1
                        strcat(newFrgStr, "CColor = texture2D(uTex0,vertexTexCoord0).rgb;\n");
                        break;
                    case CCMUX_TEXEL1 :// 2
                        strcat(newFrgStr, "CColor = texture2D(uTex1,vertexTexCoord1).rgb;\n");
                        break;
                    case CCMUX_PRIMITIVE : // 3
                        strcat(newFrgStr, "CColor = uPrimColor.rgb;\n");
                        break;
                    case CCMUX_SHADE : // 4
                        strcat(newFrgStr, "CColor = vertexShadeColor.rgb;\n");
                        break;
                    case CCMUX_ENVIRONMENT : // 5
                        strcat(newFrgStr, "CColor = uEnvColor.rgb;\n");
                        break;
                    case CCMUX_SCALE : // 6
                        strcat(newFrgStr, "CColor = uChromaKeyScale;\n");
                        break;
                    case CCMUX_COMBINED_ALPHA : // 7
                        strcat(newFrgStr, "CColor = vec3(cycle1Color.a);\n");
                        break;
                    case CCMUX_TEXEL0_ALPHA : // 8
                        strcat(newFrgStr, "CColor = vec3(texture2D(uTex0,vertexTexCoord0).a);\n");
                        break;
                    case CCMUX_TEXEL1_ALPHA : // 9
                        strcat(newFrgStr, "CColor = vec3(texture2D(uTex1,vertexTexCoord1).a);\n");
                        break;
                    case CCMUX_PRIMITIVE_ALPHA : // 10
                        strcat(newFrgStr, "CColor = vec3(uPrimColor.a);\n");
                        break;
                    case CCMUX_SHADE_ALPHA : // 11
                        strcat(newFrgStr, "CColor = vec3(vertexShadeColor.a);\n");
                        break;
                    case CCMUX_ENV_ALPHA : // 12
                        strcat(newFrgStr, "CColor = vec3(uEnvColor.a);\n");
                        break;
                    case CCMUX_LOD_FRACTION : // 13
                        strcat(newFrgStr, "CColor = vec3(uLodFrac);\n");
                        break;
                    case CCMUX_PRIM_LOD_FRAC : // 14
                        strcat(newFrgStr, "CColor = vec3(uPrimLodFrac);\n");
                        break;
                    case CCMUX_K5 : // 15
                        strcat(newFrgStr, "CColor = vec3(uK5);\n");
                        break;
                    case CCMUX_0 : // 16
                    default: // everything > CCMUX_0 (16) is 0
                        strcat(newFrgStr, "CColor = vec3(0.0, 0.0, 0.0);\n");
                        break;
                }
                
                // D0 (Cycle 2)
                switch( m_sources[11] )
                {
                    case CCMUX_COMBINED : // 0
                        strcat(newFrgStr, "DColor = cycle1Color;\n");
                        break;
                    case CCMUX_TEXEL0 : // 1
                        strcat(newFrgStr, "DColor = texture2D(uTex0,vertexTexCoord0).rgb;\n");
                        break;
                    case CCMUX_TEXEL1 :// 2
                        strcat(newFrgStr, "DColor = texture2D(uTex1,vertexTexCoord1).rgb;\n");
                        break;
                    case CCMUX_PRIMITIVE : // 3
                        strcat(newFrgStr, "DColor = uPrimColor.rgb;\n");
                        break;
                    case CCMUX_SHADE : // 4
                        strcat(newFrgStr, "DColor = vertexShadeColor.rgb;\n");
                        break;
                    case CCMUX_ENVIRONMENT : // 5
                        strcat(newFrgStr, "DColor = uEnvColor.rgb;\n");
                        break;
                    case CCMUX_1 : // 6
                        strcat(newFrgStr, "DColor = vec3(1.0, 1.0, 1.0);\n");
                        break;
                    case CCMUX_0 : // 7
                    default: // everything > CCMUX_0 (7) is 0
                        strcat(newFrgStr, "DColor = vec3(0.0, 0.0, 0.0);\n");
                        break;
                }
                
                ////////////////////////////////////////////////////////////////////////////
                // Alphas (float) Cycle 2
                ////////////////////////////////////////////////////////////////////////////
                
                // a0 (Cycle 2) (same than b0 and c0 actually)
                switch( m_sources[12] )
                {
                    case ACMUX_COMBINED : // 0
                        strcat(newFrgStr, "AAlpha = cycle1Alpha;\n");
                        break;
                    case ACMUX_TEXEL0 : // 1
                        strcat(newFrgStr, "AAlpha = texture2D(uTex0,vertexTexCoord0).a;\n");
                        break;
                    case ACMUX_TEXEL1 : // 2
                        strcat(newFrgStr, "AAlpha = texture2D(uTex1,vertexTexCoord1).a;\n");
                        break;
                    case ACMUX_PRIMITIVE : // 3
                        strcat(newFrgStr, "AAlpha = uPrimColor.a;\n");
                        break;
                    case ACMUX_SHADE : // 4
                        strcat(newFrgStr, "AAlpha = vertexShadeColor.a;\n");
                        break;
                    case ACMUX_ENVIRONMENT : // 5
                        strcat(newFrgStr, "AAlpha = uEnvColor.a;\n");
                        break;
                    case ACMUX_1 : // 6
                        strcat(newFrgStr, "AAlpha = 1.0;\n");
                        break;
                    case ACMUX_0 : // 7
                    default: // everything > CCMUX_0 (7) is 0
                        strcat(newFrgStr, "AAlpha = 0.0;\n");
                        break;
                }
                
                // b0 (Cycle 2) (same than a0 and c0 actually)
                switch( m_sources[13] )
                {
                    case ACMUX_COMBINED : // 0
                        strcat(newFrgStr, "BAlpha = cycle1Alpha;\n");
                        break;
                    case ACMUX_TEXEL0 : // 1
                        strcat(newFrgStr, "BAlpha = texture2D(uTex0,vertexTexCoord0).a;\n");
                        break;
                    case ACMUX_TEXEL1 : // 2
                        strcat(newFrgStr, "BAlpha = texture2D(uTex1,vertexTexCoord1).a;\n");
                        break;
                    case ACMUX_PRIMITIVE : // 3
                        strcat(newFrgStr, "BAlpha = uPrimColor.a;\n");
                        break;
                    case ACMUX_SHADE : // 4
                        strcat(newFrgStr, "BAlpha = vertexShadeColor.a;\n");
                        break;
                    case ACMUX_ENVIRONMENT : // 5
                        strcat(newFrgStr, "BAlpha = uEnvColor.a;\n");
                        break;
                    case ACMUX_1 : // 6
                        strcat(newFrgStr, "BAlpha = 1.0;\n");
                        break;
                    case ACMUX_0 : // 7
                    default: // everything > CCMUX_0 (7) is 0
                        strcat(newFrgStr, "BAlpha = 0.0;\n");
                        break;
                }
                
                // c0 (Cycle 2) kind of "exotic"
                switch( m_sources[14] )
                {
                    case ACMUX_LOD_FRACTION : // 0
                        strcat(newFrgStr, "CAlpha = uLodFrac;\n");
                        break;
                    case ACMUX_TEXEL0 : // 1
                        strcat(newFrgStr, "CAlpha = texture2D(uTex0,vertexTexCoord0).a;\n");
                        break;
                    case ACMUX_TEXEL1 : // 2
                        strcat(newFrgStr, "CAlpha = texture2D(uTex1,vertexTexCoord1).a;\n");
                        break;
                    case ACMUX_PRIMITIVE : // 3
                        strcat(newFrgStr, "CAlpha = uPrimColor.a;\n");
                        break;
                    case ACMUX_SHADE : // 4
                        strcat(newFrgStr, "CAlpha = vertexShadeColor.a;\n");
                        break;
                    case ACMUX_ENVIRONMENT : // 5
                        strcat(newFrgStr, "CAlpha = uEnvColor.a;\n");
                        break;
                    case ACMUX_PRIM_LOD_FRAC : // 6
                        strcat(newFrgStr, "CAlpha = uPrimLodFrac;\n");
                        break;
                    case ACMUX_0 : // 7
                    default: // everything > CCMUX_0 (7) is 0
                        strcat(newFrgStr, "CAlpha = 0.0;\n");
                        break;
                }
                
                // d0 (Cycle 2) (same than a0 and b0 actually)
                switch( m_sources[15] )
                {
                    case ACMUX_COMBINED : // 0
                        strcat(newFrgStr, "DAlpha = cycle1Alpha;\n");
                        break;
                    case ACMUX_TEXEL0 : // 1
                        strcat(newFrgStr, "DAlpha = texture2D(uTex0,vertexTexCoord0).a;\n");
                        break;
                    case ACMUX_TEXEL1 : // 2
                        strcat(newFrgStr, "DAlpha = texture2D(uTex1,vertexTexCoord1).a;\n");
                        break;
                    case ACMUX_PRIMITIVE : // 3
                        strcat(newFrgStr, "DAlpha = uPrimColor.a;\n");
                        break;
                    case ACMUX_SHADE : // 4
                        strcat(newFrgStr, "DAlpha = vertexShadeColor.a;\n");
                        break;
                    case ACMUX_ENVIRONMENT : // 5
                        strcat(newFrgStr, "DAlpha = uEnvColor.a;\n");
                        break;
                    case ACMUX_1 : // 6
                        strcat(newFrgStr, "DAlpha = 1.0;\n");
                        break;
                    case ACMUX_0 : // 7
                    default: // everything > CCMUX_0 (7) is 0
                        strcat(newFrgStr, "DAlpha = 0.0;\n");
                        break;
                }
                strcat(newFrgStr, "outColor.rgb = (AColor - BColor) * CColor + DColor;\n");
                strcat(newFrgStr, "outColor.a = (AAlpha - BAlpha) * CAlpha + DAlpha;\n");
                //strcat(newFrgStr, "outColor.rgb = cycle1Color;\n");
                //strcat(newFrgStr, "outColor.a = cycle1Alpha;\n");
                break;
            }
        } // end case CYCLE_TYPE_2
        case CYCLE_TYPE_COPY :
        case CYCLE_TYPE_FILL :
        default :
            // TODO?
            break;
    } // end switch cycle type
    
    //strcat(newFrgStr, "outColor.rgb = vec3(greaterThan(outColor.rgb, vec3(1.0)));\n");
    
    genFragmentBlenderStr( newFrgStr );
    strcat( newFrgStr, fragmentShaderFooter ); // (always the same)
    
    ////////////////////////////////////////////////////////////////////////////
    // Create the program
    ////////////////////////////////////////////////////////////////////////////
    GLuint frgShader = createShader( GL_FRAGMENT_SHADER, newFrgStr );
    
    GLuint program = createProgram( m_vtxShader, frgShader );
    
    glDeleteShader(frgShader); OPENGL_CHECK_ERRORS
    
    ////////////////////////////////////////////////////////////////////////////
    // Generate and store the save ype
    ////////////////////////////////////////////////////////////////////////////
    OGLSecondShaderCombinerSaveType shaderSaveType;
    
    shaderSaveType.combineMode1  = m_combineMode1;
    shaderSaveType.combineMode2  = m_combineMode2;
    shaderSaveType.cycle_type    = gRDP.otherMode.cycle_type;
    shaderSaveType.key_enabled   = gRDP.otherMode.key_en;
    //shaderSaveType.blender       = gRDP.otherMode.blender;
    shaderSaveType.alpha_compare = gRDP.otherMode.alpha_compare;
    shaderSaveType.aa_en         = gRDP.otherMode.aa_en;
    shaderSaveType.z_cmp         = gRDP.otherMode.z_cmp;
    shaderSaveType.z_upd         = gRDP.otherMode.z_upd;
    shaderSaveType.alpha_cvg_sel = gRDP.otherMode.alpha_cvg_sel;
    shaderSaveType.cvg_x_alpha   = gRDP.otherMode.cvg_x_alpha;
    //shaderSaveType.clr_on_cvg    = gRDP.otherMode.clr_on_cvg;
    //shaderSaveType.cvg_dst       = gRDP.otherMode.cvg_dst;
    shaderSaveType.fog_enabled   = gRSP.bFogEnabled;
    shaderSaveType.fog_in_blender = gRDP.bFogEnableInBlender;
    shaderSaveType.program       = program;
    
    StoreUniformLocations( shaderSaveType );
    
    m_vGeneratedPrograms.push_back( shaderSaveType );
    
    return m_vGeneratedPrograms.size()-1; // id of the shader save type
}

GLuint COGLSecondFragmentProgramCombiner::GenerateCopyProgram()
{
    assert( gRDP.otherMode.cycle_type == CYCLE_TYPE_COPY );
    assert( m_vtxShader != CC_NULL_SHADER );
    
    newFrgStr[0] = 0;
    strcat(newFrgStr, fragmentCopyHeader);   // (always the same)
    genFragmentBlenderStr(newFrgStr);
    strcat(newFrgStr, fragmentShaderFooter); // (always the same)
    
    ////////////////////////////////////////////////////////////////////////////
    // Create the program
    ////////////////////////////////////////////////////////////////////////////
    GLuint frgShader = createShader( GL_FRAGMENT_SHADER, newFrgStr );
    
    GLuint program = createProgram( m_vtxShader, frgShader );
    
    glDeleteShader(frgShader); OPENGL_CHECK_ERRORS
    
    ////////////////////////////////////////////////////////////////////////////
    // Generate and store the save type
    ////////////////////////////////////////////////////////////////////////////
    OGLSecondShaderCombinerSaveType shaderSaveType;
    
    // (as it a copy shader, only blender values are saved)
    shaderSaveType.cycle_type    = gRDP.otherMode.cycle_type;
    shaderSaveType.alpha_compare = gRDP.otherMode.alpha_compare;
    shaderSaveType.aa_en         = gRDP.otherMode.aa_en;
    shaderSaveType.z_cmp         = gRDP.otherMode.z_cmp;
    shaderSaveType.z_upd         = gRDP.otherMode.z_upd;
    shaderSaveType.alpha_cvg_sel = gRDP.otherMode.alpha_cvg_sel;
    shaderSaveType.cvg_x_alpha   = gRDP.otherMode.cvg_x_alpha;
    shaderSaveType.fog_enabled   = gRSP.bFogEnabled;
    shaderSaveType.fog_in_blender = gRDP.bFogEnableInBlender;
    shaderSaveType.program       = program;
    
    StoreUniformLocations( shaderSaveType );
    
    m_vGeneratedPrograms.push_back( shaderSaveType );
    
    return m_vGeneratedPrograms.size()-1;
}

void COGLSecondFragmentProgramCombiner::GenerateProgramStr()
{
    /*DecodedMux &mux = *(DecodedMux*)m_pDecodedMux;

    mux.splitType[0] = mux.splitType[1] = mux.splitType[2] = mux.splitType[3] = CM_FMT_TYPE_NOT_CHECKED;
    m_pDecodedMux->Reformat(false);

    char tempstr[500], newFPBody[4092];
    bool bNeedT0 = false, bNeedT1 = false, bNeedComb2 = false;
    newFPBody[0] = 0;

    for( int cycle=0; cycle<2; cycle++ )
    {
        for( int channel=0; channel<2; channel++)
        {
            char* (*func)(uint8) = channel==0?MuxToOC:MuxToOA;
            char *dst = channel==0?(char*)"rgb":(char*)"a";
            N64CombinerType &m = mux.m_n64Combiners[cycle*2+channel];
            switch( mux.splitType[cycle*2+channel] )
            {
            case CM_FMT_TYPE_NOT_USED:
                tempstr[0] = 0;
                break;
            case CM_FMT_TYPE_D:
                sprintf(tempstr, "MOV comb.%s, %s;\n", dst, func(m.d));
                CheckFpVars(m.d, bNeedT0, bNeedT1);
                break;
            case CM_FMT_TYPE_A_MOD_C:
                sprintf(tempstr, "MUL comb.%s, %s, %s;\n", dst, func(m.a), func(m.c));
                CheckFpVars(m.a, bNeedT0, bNeedT1);
                CheckFpVars(m.c, bNeedT0, bNeedT1);
                break;
            case CM_FMT_TYPE_A_ADD_D:
                sprintf(tempstr, "ADD_SAT comb.%s, %s, %s;\n", dst, func(m.a), func(m.d));
                CheckFpVars(m.a, bNeedT0, bNeedT1);
                CheckFpVars(m.d, bNeedT0, bNeedT1);
                break;
            case CM_FMT_TYPE_A_SUB_B:
                sprintf(tempstr, "SUB comb.%s, %s, %s;\n", dst, func(m.a), func(m.b));
                CheckFpVars(m.a, bNeedT0, bNeedT1);
                CheckFpVars(m.b, bNeedT0, bNeedT1);
                break;
            case CM_FMT_TYPE_A_MOD_C_ADD_D:
                sprintf(tempstr, "MAD_SAT comb.%s, %s, %s, %s;\n", dst, func(m.a), func(m.c), func(m.d));
                CheckFpVars(m.a, bNeedT0, bNeedT1);
                CheckFpVars(m.c, bNeedT0, bNeedT1);
                CheckFpVars(m.d, bNeedT0, bNeedT1);
                break;
            case CM_FMT_TYPE_A_LERP_B_C:
                sprintf(tempstr, "LRP_SAT comb.%s, %s, %s, %s;\n", dst, func(m.c), func(m.a), func(m.b));
                CheckFpVars(m.a, bNeedT0, bNeedT1);
                CheckFpVars(m.b, bNeedT0, bNeedT1);
                CheckFpVars(m.c, bNeedT0, bNeedT1);
                //sprintf(tempstr, "SUB comb.%s, %s, %s;\nMAD_SAT comb.%s, comb, %s, %s;\n", dst, func(m.a), func(m.b), dst, func(m.c), func(m.b));
                break;
            default:
                sprintf(tempstr, "SUB comb2.%s, %s, %s;\nMAD_SAT comb.%s, comb2, %s, %s;\n", dst, func(m.a), func(m.b), dst, func(m.c), func(m.d));
                CheckFpVars(m.a, bNeedT0, bNeedT1);
                CheckFpVars(m.b, bNeedT0, bNeedT1);
                CheckFpVars(m.c, bNeedT0, bNeedT1);
                CheckFpVars(m.d, bNeedT0, bNeedT1);
                bNeedComb2 = true;
                break;
            }
            strcat(newFPBody, tempstr);
        }
    }

    strcpy(oglNewFP, "!!ARBfp1.0\n");
    strcat(oglNewFP, "#Declarations\n");
    if (gRDP.bFogEnableInBlender && gRSP.bFogEnabled)
        strcat(oglNewFP, "OPTION ARB_fog_linear;\n");
    if (bNeedT0)
        strcat(oglNewFP, "TEMP t0;\n");
    if (bNeedT1)
        strcat(oglNewFP, "TEMP t1;\n");
    strcat(oglNewFP, "TEMP comb;\n");
    if (bNeedComb2)
        strcat(oglNewFP, "TEMP comb2;\n");
    strcat(oglNewFP, "#Instructions\n");
    if (bNeedT0)
        strcat(oglNewFP, "TEX t0, fragment.texcoord[0], texture[0], 2D;\n");
    if (bNeedT1)
        strcat(oglNewFP, "TEX t1, fragment.texcoord[1], texture[1], 2D;\n");
    strcat(oglNewFP, "# N64 cycle 1, result is in comb\n");

    strcat(oglNewFP, newFPBody);

    strcat(oglNewFP, "MOV result.color, comb;\n");
    strcat(oglNewFP, "END\n\n");*/
}

int COGLSecondFragmentProgramCombiner::ParseDecodedMux()
{
    /*

    OGLShaderCombinerSaveType res;

    pglGenProgramsARB( 1, &res.programID);
    OPENGL_CHECK_ERRORS;
    pglBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, res.programID);
    OPENGL_CHECK_ERRORS;
    GenerateProgramStr();

    pglProgramStringARB( GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, strlen(oglNewFP), oglNewFP);
    OPENGL_CHECK_ERRORS;
    //pglProgramStringARB(   GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, strlen(oglFPTest), oglFPTest);

    if (glGetError() != 0)
    {
        GLint position;
#ifdef DEBUGGER
        char *str = (char*)glGetString(GL_PROGRAM_ERROR_STRING_ARB);
#endif
        glGetIntegerv( GL_PROGRAM_ERROR_POSITION_ARB, &position);
        if( position >= 0 )
        {
#ifdef DEBUGGER
            if( m_lastIndex >= 0 ) COGLColorCombiner4::DisplaySimpleMuxString();
            DebugMessage(M64MSG_ERROR, "%s - %s", str, oglNewFP+position);
#endif
            glDisable(GL_FRAGMENT_PROGRAM_ARB);
            return COGLColorCombiner4::ParseDecodedMux();
        }
    }

    glEnable(GL_FRAGMENT_PROGRAM_ARB);
    OPENGL_CHECK_ERRORS;
    res.dwMux0 = m_pDecodedMux->m_dwMux0;
    res.dwMux1 = m_pDecodedMux->m_dwMux1;
    res.fogIsUsed = gRDP.bFogEnableInBlender && gRSP.bFogEnabled;

    m_vCompiledShaders.push_back(res);
    m_lastIndex = m_vCompiledShaders.size()-1;

    return m_lastIndex;*/
    return 0;
}

// Set vertex attribute pointers.
// The program must be bind before use this.
void COGLSecondFragmentProgramCombiner::GenerateCombinerSetting()
{
    glEnableVertexAttribArray(VS_POSITION);
    OPENGL_CHECK_ERRORS;
    glVertexAttribPointer(VS_POSITION,4,GL_FLOAT,GL_FALSE,sizeof(float)*5,&(g_vtxProjected5[0][0]));
    OPENGL_CHECK_ERRORS;

    glEnableVertexAttribArray(VS_TEXCOORD0);
    OPENGL_CHECK_ERRORS;
    glVertexAttribPointer(VS_TEXCOORD0,2,GL_FLOAT,GL_FALSE, sizeof( TLITVERTEX ), &(g_vtxBuffer[0].tcord[0].u));
    OPENGL_CHECK_ERRORS;

    glEnableVertexAttribArray(VS_TEXCOORD1);
    OPENGL_CHECK_ERRORS;
    glVertexAttribPointer(VS_TEXCOORD1,2,GL_FLOAT,GL_FALSE, sizeof( TLITVERTEX ), &(g_vtxBuffer[0].tcord[1].u));
    OPENGL_CHECK_ERRORS;

    glEnableVertexAttribArray(VS_COLOR);
    OPENGL_CHECK_ERRORS;
    glVertexAttribPointer(VS_COLOR, 4, GL_UNSIGNED_BYTE,GL_TRUE, sizeof(uint8)*4, &(g_oglVtxColors[0][0]) );
    OPENGL_CHECK_ERRORS;

    glEnableVertexAttribArray(VS_FOG);
    OPENGL_CHECK_ERRORS;
    glVertexAttribPointer(VS_FOG,1,GL_FLOAT,GL_FALSE,sizeof(float)*5,&(g_vtxProjected5[0][4]));
    OPENGL_CHECK_ERRORS;
}

// Bind various uniforms
void COGLSecondFragmentProgramCombiner::GenerateCombinerSettingConstants( int shaderId )
{
    assert( shaderId >= 0 );
    
    const OGLSecondShaderCombinerSaveType saveType = m_vGeneratedPrograms[ shaderId ];
    
    //GLint curProg;
    //glGetIntegerv(GL_CURRENT_PROGRAM, &curProg);
    //printf("bind shaderSaveType.fogMaxMinLoc %d %d %d %d %d\n", shaderId, saveType.fogMaxMinLoc, saveType.program, m_currentProgram, curProg);
    
    // Vertex shader
    if( saveType.fogMaxMinLoc != CC_INACTIVE_UNIFORM ) {
        glUniform2f( saveType.fogMaxMinLoc, gRSPfFogMin ,
                                            gRSPfFogMax );
        OPENGL_CHECK_ERRORS;
    }
    
    // Fragment shader
    if(    saveType.blendColorLoc != CC_INACTIVE_UNIFORM ) {
        glUniform4f( saveType.blendColorLoc, gRDP.fvBlendColor[0],
                                             gRDP.fvBlendColor[1],
                                             gRDP.fvBlendColor[2],
                                             gRDP.fvBlendColor[3]);
        OPENGL_CHECK_ERRORS
    }
    if(    saveType.primColorLoc != CC_INACTIVE_UNIFORM ) {
        glUniform4f( saveType.primColorLoc, gRDP.fvPrimitiveColor[0],
                                            gRDP.fvPrimitiveColor[1],
                                            gRDP.fvPrimitiveColor[2],
                                            gRDP.fvPrimitiveColor[3]);
        OPENGL_CHECK_ERRORS
    }
    
    if( saveType.envColorLoc != CC_INACTIVE_UNIFORM ) {
        glUniform4f( saveType.envColorLoc, gRDP.fvEnvColor[0],
                                           gRDP.fvEnvColor[1],
                                           gRDP.fvEnvColor[2],
                                           gRDP.fvEnvColor[3]);
        OPENGL_CHECK_ERRORS
    }
    
    if( saveType.chromaKeyCenterLoc != CC_INACTIVE_UNIFORM ) {
        glUniform3f( saveType.chromaKeyCenterLoc, gRDP.keyCenterR/255.0f,
                                                  gRDP.keyCenterG/255.0f,
                                                  gRDP.keyCenterB/255.0f);
        OPENGL_CHECK_ERRORS
    }
    
    if( saveType.chromaKeyScaleLoc != CC_INACTIVE_UNIFORM ) {
        glUniform3f( saveType.chromaKeyScaleLoc, gRDP.keyScaleR/255.0f,
                                                 gRDP.keyScaleG/255.0f,
                                                 gRDP.keyScaleB/255.0f);
        OPENGL_CHECK_ERRORS
    }
    
    if( saveType.chromaKeyWidthLoc != CC_INACTIVE_UNIFORM ) {
        glUniform3f( saveType.chromaKeyWidthLoc, gRDP.keyWidthR/255.0f,
                                                 gRDP.keyWidthG/255.0f,
                                                 gRDP.keyWidthB/255.0f);
        OPENGL_CHECK_ERRORS
    }

    if( saveType.lodFracLoc != CC_INACTIVE_UNIFORM ) {
        glUniform1f( saveType.lodFracLoc, gRDP.LODFrac/255.0f );
        OPENGL_CHECK_ERRORS
    }
    
    if( saveType.primLodFracLoc != CC_INACTIVE_UNIFORM ) {
        glUniform1f( saveType.primLodFracLoc, gRDP.primLODFrac/255.0f );
        OPENGL_CHECK_ERRORS
    }
    
    if( saveType.k5Loc != CC_INACTIVE_UNIFORM ) {
        glUniform1f( saveType.k5Loc, gRDP.K5/255.0f );
        OPENGL_CHECK_ERRORS
    }
    
    if( saveType.k4Loc != CC_INACTIVE_UNIFORM ) {
        glUniform1f( saveType.k4Loc, gRDP.K4/255.0f );
        OPENGL_CHECK_ERRORS
    }
    
    if( saveType.tex0Loc != CC_INACTIVE_UNIFORM ) {
        glUniform1i( saveType.tex0Loc,0 );
        OPENGL_CHECK_ERRORS
    }
    
    if( saveType.tex1Loc != CC_INACTIVE_UNIFORM ) {
        glUniform1i( saveType.tex1Loc,1 );
        OPENGL_CHECK_ERRORS
    }
    
    if( saveType.fogColorLoc != CC_INACTIVE_UNIFORM ) {
        glUniform4f( saveType.fogColorLoc, gRDP.fvFogColor[0],
                                           gRDP.fvFogColor[1],
                                           gRDP.fvFogColor[2],
                                           gRDP.fvFogColor[3]);
        OPENGL_CHECK_ERRORS;
    }
}

//////////////////////////////////////////////////////////////////////////
void COGLSecondFragmentProgramCombiner::InitCombinerCycle12(void)
{
    /*bool combinerIsChanged = false;

    if( m_pDecodedMux->m_dwMux0 != m_dwLastMux0 || m_pDecodedMux->m_dwMux1 != m_dwLastMux1 || m_lastIndex < 0 )
    {
        combinerIsChanged = true;
        m_lastIndex = FindCompiledMux();
        if( m_lastIndex < 0 )       // Can not found
        {
            m_lastIndex = ParseDecodedMux();
        }

        m_dwLastMux0 = m_pDecodedMux->m_dwMux0;
        m_dwLastMux1 = m_pDecodedMux->m_dwMux1;
    }


    GenerateCombinerSettingConstants(m_lastIndex);
    if( m_bCycleChanged || combinerIsChanged || gRDP.texturesAreReloaded || gRDP.colorsAreReloaded )
    {
        if( m_bCycleChanged || combinerIsChanged )
        {
            GenerateCombinerSettingConstants(m_lastIndex);
            GenerateCombinerSetting(m_lastIndex);
        }
        else if( gRDP.colorsAreReloaded )
        {
            GenerateCombinerSettingConstants(m_lastIndex);
        }

        m_pOGLRender->SetAllTexelRepeatFlag();

        gRDP.colorsAreReloaded = false;
        gRDP.texturesAreReloaded = false;
    }
    else
    {
        m_pOGLRender->SetAllTexelRepeatFlag();
    }*/
    
    /*float *pf;
    pf = GetEnvColorfv();
    pglProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 1, pf);
    OPENGL_CHECK_ERRORS;
    pf = GetPrimitiveColorfv();
    pglProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 2, pf);
    OPENGL_CHECK_ERRORS;

    float frac = gRDP.LODFrac / 255.0f;
    float tempf[4] = {frac,frac,frac,frac};
    pglProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 3, tempf);
    OPENGL_CHECK_ERRORS;

    float frac2 = gRDP.primLODFrac / 255.0f;
    float tempf2[4] = {frac2,frac2,frac2,frac2};
    pglProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 4, tempf2);
    OPENGL_CHECK_ERRORS;

    float tempf3[4] = {0,0,0,0};
    pglProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 0, tempf3);
    OPENGL_CHECK_ERRORS;
    pglProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 6, tempf3);
    OPENGL_CHECK_ERRORS;*/

    bool combinerIsChanged = false;
    
    // this statement is just an optimization and could be removed for tests
    //if( m_pDecodedMux->m_dwMux0 != m_dwLastMux0 || m_pDecodedMux->m_dwMux1 != m_dwLastMux1 )
    //{
        combinerIsChanged = true;
        /*m_lastIndex = FindCompiledMux();
        if( m_lastIndex < 0 )       // Can not found
        {
            m_lastIndex = ParseDecodedMux();
        }*/
        
        // Look if we already have a compiled program for the current state.
        int shaderId = FindCompiledShaderId();
        
        if( shaderId == -1 ) {
            shaderId = GenerateCycle12Program();
        }
        
        GLuint program = m_vGeneratedPrograms[shaderId].program;
        if( program != m_currentProgram ) {
            glUseProgram( program );
            OPENGL_CHECK_ERRORS
            m_currentProgram = program;
        }

    //}


    GenerateCombinerSettingConstants( shaderId );
    if( m_bCycleChanged || combinerIsChanged || gRDP.texturesAreReloaded || gRDP.colorsAreReloaded )
    {
        if( m_bCycleChanged || combinerIsChanged )
        {
            GenerateCombinerSettingConstants( shaderId );
            GenerateCombinerSetting();
        }
        else if( gRDP.colorsAreReloaded )
        {
            GenerateCombinerSettingConstants( shaderId );
        }

        m_pOGLRender->SetAllTexelRepeatFlag();

        gRDP.colorsAreReloaded = false;
        gRDP.texturesAreReloaded = false;
    }
    else
    {
        m_pOGLRender->SetAllTexelRepeatFlag();
    }
    OPENGL_CHECK_ERRORS
}

// Store every uniform locations on the given shader
// We don't know if every of them are used in the program so some of them will return -1 but we don't care
// Note: glGetUniformLocation doesn't need glUseProgram to work.
void COGLSecondFragmentProgramCombiner::StoreUniformLocations( OGLSecondShaderCombinerSaveType &saveType )
{
    assert( saveType.program != CC_NULL_PROGRAM );
    
    saveType.fogMaxMinLoc       = glGetUniformLocation( saveType.program, "uFogMinMax"       );
    saveType.blendColorLoc      = glGetUniformLocation( saveType.program, "uBlendColor"      );
    saveType.primColorLoc       = glGetUniformLocation( saveType.program, "uPrimColor"       );
    saveType.envColorLoc        = glGetUniformLocation( saveType.program, "uEnvColor"        );
    saveType.chromaKeyCenterLoc = glGetUniformLocation( saveType.program, "uChromaKeyCenter" );
    saveType.chromaKeyScaleLoc  = glGetUniformLocation( saveType.program, "uChromaKeyScale"  );
    saveType.chromaKeyWidthLoc  = glGetUniformLocation( saveType.program, "uChromaKeyWidth"  );
    saveType.lodFracLoc         = glGetUniformLocation( saveType.program, "uLodFrac"         );
    saveType.primLodFracLoc     = glGetUniformLocation( saveType.program, "uPrimLodFrac"     );
    saveType.k5Loc              = glGetUniformLocation( saveType.program, "uK5"              );
    saveType.k4Loc              = glGetUniformLocation( saveType.program, "uK4"              );
    saveType.tex0Loc            = glGetUniformLocation( saveType.program, "uTex0"            );
    saveType.tex1Loc            = glGetUniformLocation( saveType.program, "uTex1"            );
    saveType.fogColorLoc        = glGetUniformLocation( saveType.program, "uFogColor"        );
}

// Return a shader id that match the current state in the current compiled shader "database".
// Return -1 if no shader is found
int COGLSecondFragmentProgramCombiner::FindCompiledShaderId()
{
    int shaderId = -1;
    for( size_t i=0; i<m_vGeneratedPrograms.size(); i++ )
    {
        OGLSecondShaderCombinerSaveType* saveType = &m_vGeneratedPrograms[i];
        switch( gRDP.otherMode.cycle_type ) {
            case CYCLE_TYPE_1 :
            case CYCLE_TYPE_2 :
                if(    saveType->combineMode1  == m_combineMode1
                    && saveType->combineMode2  == m_combineMode2
                    && saveType->cycle_type    == gRDP.otherMode.cycle_type // 1 or 2?
                    && saveType->key_enabled   == gRDP.otherMode.key_en
                    // Blender
                    && saveType->alpha_compare == gRDP.otherMode.alpha_compare
                    && saveType->aa_en         == gRDP.otherMode.aa_en
                    && saveType->z_cmp         == gRDP.otherMode.z_cmp
                    && saveType->z_upd         == gRDP.otherMode.z_upd
                    && saveType->alpha_cvg_sel == gRDP.otherMode.alpha_cvg_sel
                    && saveType->cvg_x_alpha   == gRDP.otherMode.cvg_x_alpha
                    && saveType->fog_enabled   == gRSP.bFogEnabled
                    && saveType->fog_in_blender == gRDP.bFogEnableInBlender
                    ) shaderId = i;
                break;
            case CYCLE_TYPE_COPY : // don't care about Color Combiner stuff, just Blender
                 if(   saveType->cycle_type    == CYCLE_TYPE_COPY
                    && saveType->alpha_compare == gRDP.otherMode.alpha_compare
                    && saveType->aa_en         == gRDP.otherMode.aa_en
                    && saveType->z_cmp         == gRDP.otherMode.z_cmp
                    && saveType->z_upd         == gRDP.otherMode.z_upd
                    && saveType->alpha_cvg_sel == gRDP.otherMode.alpha_cvg_sel
                    && saveType->cvg_x_alpha   == gRDP.otherMode.cvg_x_alpha
                    && saveType->fog_enabled   == gRSP.bFogEnabled
                    && saveType->fog_in_blender == gRDP.bFogEnableInBlender
                    ) shaderId = i;
                break;
            case CYCLE_TYPE_FILL :
                DebugMessage(M64MSG_WARNING, "Lookup for a cycle type Fill shader. It should never happend.");
                break;
            default :
                break;
        } // end switch cycle type
    } // end loop
    
    return shaderId;
}

#ifdef DEBUGGER
void COGLSecondFragmentProgramCombiner::DisplaySimpleMuxString(void)
{
    COGLColorCombiner::DisplaySimpleMuxString();
    DecodedMux &mux = *(DecodedMux*)m_pDecodedMux;
    mux.Reformat(false);
    GenerateProgramStr();
    //sprintf(oglNewFP, oglFP, 
    //  MuxToOC(mux.aRGB0), MuxToOC(mux.bRGB0), MuxToOC(mux.cRGB0), MuxToOC(mux.dRGB0),
    //  MuxToOA(mux.aA0), MuxToOA(mux.bA0), MuxToOA(mux.cA0), MuxToOA(mux.dA0),
    //  MuxToOC(mux.aRGB1), MuxToOC(mux.bRGB1), MuxToOC(mux.cRGB1), MuxToOC(mux.dRGB1),
    //  MuxToOA(mux.aA1), MuxToOA(mux.bA1), MuxToOA(mux.cA1), MuxToOA(mux.dA1)
    //  );

    TRACE0("OGL Fragment Program:");
    TRACE0(oglNewFP);
}
#endif

