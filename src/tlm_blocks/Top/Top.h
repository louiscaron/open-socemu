#ifndef TOP_H_
#define TOP_H_

// necessary define for processes in simple_target_socket
#define SC_INCLUDE_DYNAMIC_PROCESSES

// obvious include
#include "systemc"

// command line parameters
#include "Parameters.h"

// generic blocks
#include "Cpu/Cpu.h"
#include "Generic/Memory/Memory.h"
// system specific blocks
#include "Top/Bus/Bus.h"
#include "Top/IntCtrl/IntCtrl.h"
#include "Top/Dummy/Dummy.h"

// necessary for dummy target
#include "tlm_utils/simple_target_socket.h"

// Definition
#define TOP_NUM_MEMORIES 1

/// "Top" platform module
struct Top : sc_core::sc_module
{
    /// Bus instance (specify the number of masters and slaves)
    Bus<1,TOP_NUM_MEMORIES+2>* bus;
    /// CPU instance
    Cpu* cpu;
    /// CPU with debugger instance
    BusMaster* cpubase;
    /// Interrupt controller instance
    IntCtrl* intctrl;
    /// Dummy peripheral instance
    Dummy* dummy;
    /// Memory instances array
    Memory* memory[TOP_NUM_MEMORIES];

    /** Constructor of the top level module
     * @param[in] name Name of the module
     * @param[in] parameters Command line parameters
     * @param[in] config Parameters of the current block (and sub-blocks)
     */
    Top(sc_core::sc_module_name name, Parameters& parameters, MSP& config);
};

#endif /*TOP_H_*/
