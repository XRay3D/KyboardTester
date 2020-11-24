#ifndef MEASURINGINTERFACE_H
#define MEASURINGINTERFACE_H

#include "amk_tester.h"
#include "grbl.h"

class Interface {
    static inline AmkTester* m_tester = nullptr;
    static inline GRBL* m_grbl = nullptr;
    static inline QThread thread;
    static inline QSemaphore semafore;

public:
    Interface();

    ~Interface();

    static AmkTester* tester() { return m_tester; }
    static GRBL* grbl() { return m_grbl; }
};

#endif // MEASURINGINTERFACE_H
