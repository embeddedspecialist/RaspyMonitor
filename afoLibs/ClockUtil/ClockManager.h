/* 
 * File:   ClockManager.h
 * Author: amirrix
 *
 * Created on 28 gennaio 2010, 9.30
 */

#ifndef _CLOCKMANAGER_H
#define	_CLOCKMANAGER_H

#include <vector>
#include <iostream>
#include <pthread.h>
#include <vector>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

using namespace std;
//La massima variazione di clock che posso ammettere dall'ultima volta che ho interrogato
//la imposto a 10 minuti
#define MAX_CLOCK_SKEW 600
class ClockManager {
public:
    ClockManager();

    virtual ~ClockManager();

    /**
     * Aggiunge un clock e ne ritorna l'indice
    */
    unsigned int GetClock();

    /**
     * Torna -1 se errore
     * */
    int GetTimeElapsed(int clockIndex, bool reset = true);

    bool ResetClock(int clockIndex);

    bool ResetAllClocks();

private:

    vector<struct timeval> m_ClocksVector;

    struct timeval m_LastClockVal;

};

#endif	/* _CLOCKMANAGER_H */

