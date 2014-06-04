/***************************************************************************
 *   Copyright (C) 2006 by Alessandro Mirri   *
 *   alessandro.mirri@newtohm.it   *
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;                     *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 *************************************************************************/
#ifndef STTHU_H
#define STTHU_H

#include "vcontroller.h"
#include "ds2438.h"

#define THU_TEMPERATURE_IDX 0
#define THU_HUMIDITY_IDX 1
#define THU_RIT_IDX 2

/**
This class is a wrapper for a generic Analog Input device based on the Dallas-Maxim ds2438
 NAME:Thu,ISRIT:1,SP:20,ISRELATIVE:0,RIT:3,COMMENT:
 * Dove:
 * ISRIT: Indica che funge anche da ritaratore
 * SP: Indica il SetPoint remoto nel caso di ritaratore relativo
 * ISRELATIVE: Se 1 indica che il ritaratore è relativo
 * RIT: Indica l'indice di ritaratura nel caso di ritaratore relativo
	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CThu:public CVController
{
    public:
        CThu(const char* configString = NULL);
        ~CThu();

        bool Update2(bool updateData);
        bool Update(bool updateData);

        bool GetSetPointThu() { return m_SetPointThu; };
        bool GetTempThu() { return m_TempThu; };
        bool GetHumThu() { return m_HumThu; };

        bool SetInputDevice(CVDevice* inDevice);

        bool SetVal(float val){ return "<ERROR TYPE=\"Function NOT IMPLEMENTED\" />";};
        bool VerifyIOPresence(){ return "<ERROR TYPE=\"Function NOT IMPLEMENTED\" />";};
        CString GetSpontaneousData(int lParam = 0) { return "<ERROR TYPE=\"Function NOT IMPLEMENTED\" />";};

        void AcquireSetPoint(int currentRegister);

    private:
        CVDevice* m_InDevice;

        bool m_IsRit;
        bool m_IsRelative;

        float m_SetPoint;
        float m_Rit;

        float m_SetPointThu;
        float m_TempThu;
        float m_HumThu;
};


#endif
