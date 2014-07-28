/*
Copyright (C) 2002 Rice1964

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

#include "Combiner.h"
#include "Config.h"
#include "RenderBase.h"

//static BOOL g_bHiliteRGBAHack = FALSE;


#ifdef DEBUGGER
const char *constStrs[] = {
    "MUX_0",
    "MUX_1",
    "MUX_COMBINED",
    "MUX_TEXEL0",
    "MUX_TEXEL1",
    "MUX_PRIM",
    "MUX_SHADE",
    "MUX_ENV",
    "MUX_COMBALPHA",
    "MUX_T0_ALPHA",
    "MUX_T1_ALPHA",
    "MUX_PRIM_ALPHA",
    "MUX_SHADE_ALPHA",
    "MUX_ENV_ALPHA",
    "MUX_LODFRAC",
    "MUX_PRIMLODFRAC",
    "MUX_K5",
    "MUX_UNK",
};

const char *cycleTypeStrs[] = {
    "1 Cycle",
    "2 Cycle",
    "Copy Mode",
    "Fill Mode"
};

const char* constStr(uint32 op)
{
if(op<=MUX_UNK)
    return constStrs[op];
else
   return "Invalid-Const";
}
#endif

void swap(uint8 &a, uint8 &b)
{
    uint8 c=a;
    a=b;
    b=c;
}

//*****************************************************************************
//
//*****************************************************************************
bool    gUsingPrimColour = false;
bool    gUsingEnvColour = false;

int CountTexel1Cycle(N64CombinerType &m)
{
    int hasTexel[2];
    uint8 *p = (uint8*)&m;

    for( int i=0; i<2; i++)
    {
        hasTexel[i]=0;
        for( int j=0; j<4; j++)
        {
            if( (p[j]&MUX_MASK) == MUX_TEXEL0+i )
            {
                hasTexel[i]=1;
                break;
            }
        }
    }

    return hasTexel[0]+hasTexel[1];
}

uint32 GetTexelNumber(N64CombinerType &m)
{
    if( (m.a&MUX_MASK) == MUX_TEXEL1 || (m.b&MUX_MASK) == MUX_TEXEL1 || (m.c&MUX_MASK) == MUX_TEXEL1  || (m.d&MUX_MASK) == MUX_TEXEL1 )
        return TEX_1;
    else
        return TEX_0;
}

bool IsTxtrUsed(N64CombinerType &m)
{
    if( (m.a&MUX_MASK) == MUX_TEXEL1 || (m.b&MUX_MASK) == MUX_TEXEL1 || (m.c&MUX_MASK) == MUX_TEXEL1  || (m.d&MUX_MASK) == MUX_TEXEL1 )
        return true;
    if( (m.a&MUX_MASK) == MUX_TEXEL0 || (m.b&MUX_MASK) == MUX_TEXEL0 || (m.c&MUX_MASK) == MUX_TEXEL0  || (m.d&MUX_MASK) == MUX_TEXEL0 )
        return true;
    else
        return false;
}

//========================================================================

void CColorCombiner::InitCombinerMode(void)
{
#ifdef DEBUGGER
    LOG_UCODE(cycleTypeStrs[gRDP.otherMode.cycle_type]);
    if( debuggerDropDecodedMux )
    {
        UpdateCombiner(m_pDecodedMux->m_dwMux0, m_pDecodedMux->m_dwMux1);
    }
#endif

    if( currentRomOptions.bNormalCombiner )
    {
        DisableCombiner();
    }
    else if( gRDP.otherMode.cycle_type  == CYCLE_TYPE_COPY )
    {
        InitCombinerCycleCopy();
        m_bCycleChanged = true;
    }
    else if ( gRDP.otherMode.cycle_type == CYCLE_TYPE_FILL )
    //else if ( gRDP.otherMode.cycle_type == CYCLE_TYPE_FILL && gRSP.ucode != 5 )   //hack
    {
        InitCombinerCycleFill();
        m_bCycleChanged = true;
    }
    else
    {
        InitCombinerCycle12();
        m_bCycleChanged = false;
    }
}


bool bConkerHideShadow=false;
void CColorCombiner::UpdateCombiner(uint32 dwMux0, uint32 dwMux1)
{
#ifdef DEBUGGER
    if( debuggerDropDecodedMux )
    {
        debuggerDropDecodedMux = false;
        m_pDecodedMux->m_dwMux0 = m_pDecodedMux->m_dwMux1 = 0;
        m_DecodedMuxList.clear();
    }
#endif

    DecodedMux &m_decodedMux = *m_pDecodedMux;
    if( m_decodedMux.m_dwMux0 != dwMux0 || m_decodedMux.m_dwMux1 != dwMux1 )
    {
        if( options.enableHackForGames == HACK_FOR_DR_MARIO )
        {
            // Hack for Dr. Mario
            if( dwMux1 == 0xfffcf239 && 
                ((m_decodedMux.m_dwMux0 == dwMux0 && dwMux0 == 0x00ffffff && 
                m_decodedMux.m_dwMux1 != dwMux1 && m_decodedMux.m_dwMux1 == 0xfffcf279 ) || 
                (m_decodedMux.m_dwMux0 == 0x00ffb3ff && m_decodedMux.m_dwMux1 == 0xff64fe7f && dwMux0 == 0x00ffffff ) ))
            {
                //dwMux1 = 0xffcf23A;
                dwMux1 = 0xfffcf438;
            }
        }
        uint64 mux64 = (((uint64)dwMux1)<<32)+dwMux0;
        int index=m_DecodedMuxList.find(mux64);

        if( options.enableHackForGames == HACK_FOR_CONKER )
        {
            // Conker's shadow, to disable the shadow
            //Mux=0x00ffe9ff    Used in CONKER BFD
            //Color0: (0 - 0) * 0 + SHADE
            //Color1: (0 - 0) * 0 + SHADE
            //Alpha0: (1 - TEXEL0) * SHADE + 0
            //Alpha1: (1 - TEXEL0) * SHADE + 0              
            if( dwMux1 == 0xffd21f0f && dwMux0 == 0x00ffe9ff )
            {
                bConkerHideShadow = true;
            }
            else
            {
                bConkerHideShadow = false;
            }
        }

        if( index >= 0 )
        {
            m_decodedMux = m_DecodedMuxList[index];
        }
        else
        {
            m_decodedMux.Decode(dwMux0, dwMux1);
            m_decodedMux.splitType[0] = CM_FMT_TYPE_NOT_CHECKED;
            m_decodedMux.splitType[1] = CM_FMT_TYPE_NOT_CHECKED;
            m_decodedMux.splitType[2] = CM_FMT_TYPE_NOT_CHECKED;
            m_decodedMux.splitType[3] = CM_FMT_TYPE_NOT_CHECKED;

            m_decodedMux.Hack();

            m_decodedMux.Simplify();
            if( m_supportedStages>1)    
                m_decodedMux.SplitComplexStages();
            
            m_DecodedMuxList.add(m_decodedMux.m_u64Mux, *m_pDecodedMux);
#ifdef DEBUGGER
            if( logCombiners ) 
            {
                TRACE0("Add a new mux");
                DisplayMuxString();
            }
#endif
        }

        m_bTex0Enabled = m_decodedMux.m_bTexel0IsUsed;
        m_bTex1Enabled = m_decodedMux.m_bTexel1IsUsed;
        m_bTexelsEnable = m_bTex0Enabled||m_bTex1Enabled;
    }
}

// rgb0   = (A0 - B0) * C0 + D0
// rgb1   = (A1 - B1) * C1 + D1
// alpha0 = (a0 - b0) * c0 + d0
// alpha1 = (a1 - b1) * c1 + d1
void CColorCombiner::SetCombineMode(uint32 dwMux0, uint32 dwMux1)
{
    uint8 A0 = (uint8)((dwMux0 >> 20) & 0xF);
    uint8 B0 = (uint8)((dwMux1 >> 28) & 0xF);
    uint8 C0 = (uint8)((dwMux0 >> 15) & 0x1F);
    uint8 D0 = (uint8)((dwMux1 >> 15) & 0x7);
    uint8 a0 = (uint8)((dwMux0 >> 12) & 0x7);
    uint8 b0 = (uint8)((dwMux1 >> 12) & 0x7);
    uint8 c0 = (uint8)((dwMux0 >> 9)  & 0x7);
    uint8 d0 = (uint8)((dwMux1 >> 9)  & 0x7);
    
    uint8 A1 = (uint8)((dwMux0 >> 5)  & 0xF);
    uint8 B1 = (uint8)((dwMux1 >> 24) & 0xF);
    uint8 C1 = (uint8)((dwMux0 >> 0)  & 0x1F);
    uint8 D1 = (uint8)((dwMux1 >> 6)  & 0x7);
    uint8 a1 = (uint8)((dwMux1 >> 21) & 0x7);
    uint8 b1 = (uint8)((dwMux1 >> 3)  & 0x7);
    uint8 c1 = (uint8)((dwMux1 >> 18) & 0x7);
    uint8 d1 = (uint8)((dwMux1 >> 0)  & 0x7);
    
    // TODO use an array is not very nice, we should find something better.
    m_sources[0] = A0;
    m_sources[1] = B0;
    m_sources[2] = C0;
    m_sources[3] = D0;
    m_sources[4] = a0;
    m_sources[5] = b0;
    m_sources[6] = c0;
    m_sources[7] = d0;
    m_sources[8] = A1;
    m_sources[9] = B1;
    m_sources[10] = C1;
    m_sources[11] = D1;
    m_sources[12] = a1;
    m_sources[13] = b1;
    m_sources[14] = c1;
    m_sources[15] = d1;

    m_combineMode1 = dwMux0;
    m_combineMode2 = dwMux1;
}


#ifdef DEBUGGER
void CColorCombiner::DisplayMuxString(void)
{
    if( gRDP.otherMode.cycle_type == CYCLE_TYPE_COPY)
    {
        TRACE0("COPY Mode\n");
    }   
    else if( gRDP.otherMode.cycle_type == CYCLE_TYPE_FILL)
    {
        TRACE0("FILL Mode\n");
    }

    m_pDecodedMux->DisplayMuxString("Used");
}

void CColorCombiner::DisplaySimpleMuxString(void)
{
    m_pDecodedMux->DisplaySimpliedMuxString("Used");
}
#endif

