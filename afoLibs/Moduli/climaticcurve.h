/***************************************************************************
 *   Copyright (C) 2006 by Alessandro Mirri                                *
 *   alessandro.mirri@newtohm.it                                           *
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;                     *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 *************************************************************************/
 #ifndef STDCLIMATICCURVE_H
#define STDCLIMATICCURVE_H

#include <vcoordinator.h>
#include "temperaturecontroller.h"
#include "c3pointctrl.h"
#include "analogIO.h"

using namespace std;
typedef struct
{
    float tExt;
    float tMnd;
}t_ClimaticPoint;

/**
Coordinatore che implementa un controllo di setpoint su uno o più regolatori:
 * Ha due modalità di funzionamentourve: su curva climatica o su ON/OFF da timer, dipende dal
 * parametro Type; se Type è:
 * - compExt (compensazione esterna) legge i punti e applica la curva climatica
 * - timer  riceve dal timer l'informazione ON/OFF e applica di conseguenza i setpoint
 * - digital riceve da un digitale l'informazione ON/OFF
 * - mixedOn/Off riceve sia dal digitale che dal timer l'informazione. In questo caso viene inviato il setpoint OFF se almeno uno dei due è off

	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CClimaticCurve : public CVCoordinator
{
public:
    CClimaticCurve(const char *configString,CTimer *timer);

    ~CClimaticCurve();

    bool Update(bool updateData);
    bool Update2(bool updateData);
    bool SetVal(float val){return false;};
    
    CString GetSpontaneousData(int lParam = 0) { return "<ERROR TYPE=\"Function NOT IMPLEMENTED\" />";};
    bool ExecCommand(CXMLUtil* xmlUtil);

    bool ConnectControllers();

    vector<t_ClimaticPoint> m_ClimaticPoints;

    int m_ConfigID;

    bool UpdateSetpoint(float newSetpoint);

private:
    CTempCtrl* m_TempExt;
    CDigitalIO* m_DigitalIn;
    float m_SetpointOn, m_SetpointOff;
    float m_NewSetpoint;
    CString m_Type;

    void SaveClimaticCurve();

    bool UpdateCompExtMode();
    bool UpdateTimerMode();
    bool UpdateDigitalMode();
    bool UpdateMixedMode();

};


#endif
