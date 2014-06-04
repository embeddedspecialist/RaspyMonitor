/* 
 * File:   ClimaticCurveBlock.cpp
 * Author: amirrix
 * 
 * Created on 4 febbraio 2010, 17.23
 */

#include "ClimaticCurveBlock.h"
#include "conewireengine.h"

ClimaticCurveBlock::ClimaticCurveBlock(const char* configString) : CBlock(configString) {
    m_LibIniReader.GetConfigParamInt( configString, "ID", &m_ConfigID, -1);

    LoadPoints();
}


ClimaticCurveBlock::~ClimaticCurveBlock() {
}

void ClimaticCurveBlock::LoadPoints(){
    CIniFileHandler iniFileReader;
    CString configIDString;
    int nOfPoints = 0;

    //Leggo dal file di configurazione (changeover.ini) tutte le chiavi e le zone e le istanzio, programmo i moduli di controllo accessi
    if ( !iniFileReader.Load ( CLIMATIC_FILE ) )
    {
        cout << "Attenzione: impossibile aprire il file" <<  CLIMATIC_FILE <<"\n Il blocco" <<
                m_ConfigSubSystemIndex<<":"<<m_ConfigBlockIndex<<" sara' disabilitato" <<endl;
        return;
    }

    if (m_ConfigID <= 0)
    {
        cout << "Attenzione il blocco climatico " <<m_ConfigSubSystemIndex<<":"<<m_ConfigBlockIndex<< " NON ha il campo corretto in CONFIGID"<<endl;
        sleep(5);
        return;
    }
    else
    {
        configIDString="CLIMATE";
        configIDString+=m_ConfigID;

        if (!iniFileReader.ExistSection(configIDString))
        {
            cout << "Attenzione blocco climatico " <<m_ConfigSubSystemIndex<<":"<<m_ConfigBlockIndex<<" in climatic.ini NON esiste la sezione"<<configIDString<<endl;
            sleep(5);
            return;
        }
    }

    //Carico i punti della curva climatica
        nOfPoints = iniFileReader.GetInt("nOfPoints", configIDString,0);

        for (int i = 0; i < nOfPoints; i++)
        {
            CString pointString = "Point";
            CString pointConfig;
            t_ClimaticBlockPoint newPoint;

            pointString+=(i+1);

            pointConfig = iniFileReader.GetString(pointString, configIDString, "");

            if (pointConfig.size() == 0)
            {
                cout << "Attenzione: blocco climatico " <<m_ConfigSubSystemIndex<<":"<<m_ConfigBlockIndex<<
                        " campo "<<pointString<<" NON valido in climateCurve.ini"<<endl;
                msDelay(1000);
                continue;
            }

            //TODO da mettere protezione nel caso in cui i sottocampi non siano validi
            m_LibIniReader.GetConfigParamFloat(pointConfig.c_str(),"TEXT",&(newPoint.tExt),0.0);
            m_LibIniReader.GetConfigParamFloat(pointConfig.c_str(),"TMND",&(newPoint.tMnd),0.0);

            m_ClimaticPoints.push_back(newPoint);
        }
}

bool ClimaticCurveBlock::Update(){
    float tExt, newSetpoint;
    unsigned int tempPointIdx;
    int nOfPoints = m_ClimaticPoints.size();

    if (nOfPoints == 0)
    {
        return true;
    }

    //Leggo l'ingresso
    if ( (!IsInputConnected(0))||(!IsInputValid(0)))
    {
        SetOutputVal(0,-100.0,false);
        return false;
    }

    //Controllo la tempext in che fascia si trova e interpolo per avere il setpoint
    GetDataFromInput(0, &tExt);


    if (tExt < m_ClimaticPoints[0].tExt)
    {
        newSetpoint = m_ClimaticPoints[0].tMnd;
    }
    else if (tExt > m_ClimaticPoints[nOfPoints-1].tExt)
    {
        newSetpoint = m_ClimaticPoints[nOfPoints-1].tMnd;
    }
    else
    {
        //Qui devo ciclare per vedere tra che punti della curva sono
        for (tempPointIdx = 0; tempPointIdx<m_ClimaticPoints.size()-1;tempPointIdx++)
        {
            if ((tExt >= m_ClimaticPoints[tempPointIdx].tExt) &&
                (tExt <= m_ClimaticPoints[tempPointIdx+1].tExt)
               )
            {
                float x1,y1,x2,y2;

                x1 = m_ClimaticPoints[tempPointIdx].tExt;
                y1 = m_ClimaticPoints[tempPointIdx].tMnd;

                x2 = m_ClimaticPoints[tempPointIdx+1].tExt;
                y2 = m_ClimaticPoints[tempPointIdx+1].tMnd;

                //Punto trovato, interpolo per trovare il setpoint
                newSetpoint = ((y2-y1)/(x2-x1))*tExt + y1 - ((y2-y1)/(x2-x1))*x1;
            }
        }
    }

    return SetOutputVal(0,newSetpoint,true);
}

bool ClimaticCurveBlock::ExecCommand(CXMLUtil* xmlUtil, bool* commandRetCode){

    bool retVal = false;
    COneWireEngine *engPtr = reinterpret_cast<COneWireEngine*>(m_EngPtr);

    if (!IsCommandForMe(xmlUtil)){
        return false;
    }

    CString command = xmlUtil->GetStringParam("BLOCKEXEC");
    if (command.ToUpper() =="SETCLIMATICCURVE")
    {
        int nOfPoints = xmlUtil->GetIntParam("NOFPOINTS");

        m_ClimaticPoints.clear();

        for (int i = 0; i < nOfPoints; i++)
        {
            t_ClimaticBlockPoint newPoint;
            float tExt, tMnd;
            CString pointStr = "POINT";
            CString valString;

            pointStr+=(i+1);
            valString = xmlUtil->GetStringParam(pointStr);

            if (sscanf(valString.c_str(), "%f:%f", &tExt, &tMnd) == 2)
            {
                newPoint.tExt = tExt;
                newPoint.tMnd = tMnd;
                m_ClimaticPoints.push_back(newPoint);
            }
        }

        SaveClimaticCurve();
        retVal = true;

    } else if  (command.ToUpper() =="GETCLIMATICCURVE")
    {
        //Il formato è il seguente:
        //NOFPOINTS= POINT1=text:tmnd

        Cmd com("BLOCK");
        com.putValue("BLOCKEXEC","ClimaticCurve");
        com.putValue("ADDRESS", m_BlockAddress);
        com.putValue("NOFPOINTS",(unsigned int)m_ClimaticPoints.size());
        for (unsigned int i = 0; i < m_ClimaticPoints.size(); i++)
        {
            CString pointStr = "POINT";
            CString valString = "";
            pointStr += (i+1);
            valString+=m_ClimaticPoints.at(i).tExt;
            valString+=":";
            valString+=m_ClimaticPoints.at(i).tMnd;
            com.putValue(pointStr,valString);
        }


        if (engPtr->CheckInterfacePortsForConnection())
        {
            engPtr->WriteOnInterfacePorts(com.toString().c_str(), com.toString().size());
        }

        retVal = true;

    }

    return retVal;
}

void ClimaticCurveBlock::SaveClimaticCurve()
{
    CIniFileHandler iniFileReader;
    CString configIDString="CLIMATE";
    configIDString+=m_ConfigID;

    if (!iniFileReader.Load(CLIMATIC_FILE))
    {
        return;
    }

    iniFileReader.SetInt("nOfPoints",m_ClimaticPoints.size(),"",configIDString);

    for (unsigned int i = 0; i < m_ClimaticPoints.size(); i++)
    {
        CString pointStr = "Point";
        pointStr+=(i+1);

        CString valString = "TEXT:";
        valString+=m_ClimaticPoints.at(i).tExt;
        valString+=",TMND:";
        valString+=m_ClimaticPoints.at(i).tMnd;

        iniFileReader.SetValue(pointStr,valString,"",configIDString);
    }

    iniFileReader.Save();
}