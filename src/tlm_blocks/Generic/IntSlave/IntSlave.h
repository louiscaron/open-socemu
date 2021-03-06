#ifndef INTSLAVE_H_
#define INTSLAVE_H_

// necessary define for processes in simple_target_socket
#define SC_INCLUDE_DYNAMIC_PROCESSES

// obvious inclusion
#include "systemc"

// not so obvious inclusions
#include "tlm.h"
#include "tlm_utils/simple_target_socket.h"

// for the helper macros
#include "utils.h"

#define INTSLAVE_DEBUG_LEVEL 0

/// Macro to print debug messages
/// @param __l level of debug message (0 means always printed)
/// @param __f format of the debug string
/// @param ... variable arguments
#define INTSLAVE_TLM_DBG(__l, __f, ...)                                                 \
    do {                                                                                \
        if (INTSLAVE_DEBUG_LEVEL >= __l) {                                              \
            TLM_DBG(__f, __VA_ARGS__);                                                  \
        }                                                                               \
    } while (false)

template <typename MODULE>
struct IntSlave
{
    /** Constructor
     */
    IntSlave()
    : slave_socket("int_slave")
    {
        // register callback functions for incoming interface method calls
        slave_socket.register_b_transport(this, &IntSlave::slave_b_transport);
        slave_socket.register_nb_transport_fw(this, &IntSlave::slave_nb_transport_fw);
        slave_socket.register_get_direct_mem_ptr(this, &IntSlave::slave_get_direct_mem_ptr);
        slave_socket.register_transport_dbg(this, &IntSlave::slave_dbg_transport);
    }

    /** Constructor
     * @param[in] owner Pointer to the module that owns this instance
     * @param[in] set_cb Pointer to the method of the object handling interrupt assertion
     * @param[in] clear_cb Pointer to the method of the object handling interrupt deassertion
     * @param[in] opaque Opaque pointer passed to the methods when invoked
     */
    IntSlave(MODULE* owner,
             void (MODULE::*set_cb)(void*),
             void (MODULE::*clear_cb)(void*),
             void* opaque)
    : slave_socket("int_slave")
    {
        // register callback functions for incoming interface method calls
        slave_socket.register_b_transport(this, &IntSlave::slave_b_transport);
        slave_socket.register_nb_transport_fw(this, &IntSlave::slave_nb_transport_fw);
        slave_socket.register_get_direct_mem_ptr(this, &IntSlave::slave_get_direct_mem_ptr);
        slave_socket.register_transport_dbg(this, &IntSlave::slave_dbg_transport);
        this->init(owner, set_cb, clear_cb, opaque);
    }

    /** Initializer
     * @param[in] owner Pointer to the module that owns this instance
     * @param[in] set_cb Pointer to the method of the object handling interrupt assertion
     * @param[in] clear_cb Pointer to the method of the object handling interrupt deassertion
     * @param[in] opaque Opaque pointer passed to the methods when invoked
     */
    void
    init(MODULE* owner,
         void (MODULE::*set_cb)(void*),
         void (MODULE::*clear_cb)(void*),
         void* opaque)
    {
        m_owner = owner;
        m_set_cb = set_cb;
        m_clear_cb = clear_cb;
        m_opaque = opaque;
        m_state = false;
    }

    /** Indicate if slave socket is bound
     * @return True if slave socket is bound, false otherwise
     */
    bool
    is_bound()
    {
        return (this->slave_socket.get_base_port().get_interface() != NULL);
    }

    /** Operator & to return the reference to the slave socket
     * @return The reference to the slave socket
     */
    operator tlm::tlm_target_socket<> & ()
    {
        return this->slave_socket;
    }

    /** Operator * to return the pointer to the slave socket
     * @return The pointer to the slave socket
     */
    operator tlm::tlm_target_socket<> * ()
    {
        return &this->slave_socket;
    }

    /** Get the current interrupt state
     * @return True if interrupt is set, false otherwise
     */
    bool
    get_state()
    {
        return this->state;
    }

protected:
    /// TLM-2 slave socket, defaults to 32-bits wide, base protocol
    tlm_utils::simple_target_socket<IntSlave> slave_socket;

    /// Pointer to the owner instance of the interrupt slave
    MODULE* m_owner;

    /// Pointer to the class of the owner handling interrupt assertion
    void (MODULE::*m_set_cb)(void*);

    /// Pointer to the class of the owner handling interrupt deassertion
    void (MODULE::*m_clear_cb)(void*);

    /// Opaque parameter of the owner methods
    void* m_opaque;

    /// Current state
    bool m_state;

    /** slave_socket blocking transport method (default behavior, can be overridden)
     * @param[in, out] trans Transaction payload object, allocated by initiator, filled here
     * @param[in, out] delay Time object, allocated by initiator, filled here
     */
    void
    slave_b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay)
    {
        uint32_t* ptr = reinterpret_cast<uint32_t*>(trans.get_data_ptr());
        TLM_WORD_SANITY(trans);
        assert(trans.get_command() == tlm::TLM_WRITE_COMMAND);
        assert(trans.get_address() == 0);
        assert(*ptr < 2);

        if (*ptr)
        {
            m_state = true;
            (m_owner->*m_set_cb)(m_opaque);
        }
        else
        {
            m_state = false;
            (m_owner->*m_clear_cb)(m_opaque);
        }

        // there was no error in the processing
        trans.set_response_status(tlm::TLM_OK_RESPONSE);
    }

    /** slave_socket non-blocking forward transport method (default behavior, can be overridden)
     * @param[in, out] trans Transaction payload object, allocated by initiator, filled here
     * @param[in, out] phase Phase payload object, allocated by initiator
     * @param[in, out] delay Time object, allocated by initiator, filled here
     * @return The base protocol non blocking state
     */
    tlm::tlm_sync_enum
    slave_nb_transport_fw(tlm::tlm_generic_payload& trans,
                          tlm::tlm_phase& phase, sc_core::sc_time& delay)
    {
        SC_REPORT_FATAL("TLM-2", "Non blocking not implemented");
        return tlm::TLM_COMPLETED;
    }

    /** slave_socket direct memory access transport method
     * @param[in, out] trans Transaction payload object, allocated by initiator, filled here
     * @param[in, out] dmi_data Direct Memory Interface object
     */
    bool
    slave_get_direct_mem_ptr(tlm::tlm_generic_payload& trans,
                             tlm::tlm_dmi& dmi_data)
    {
        SC_REPORT_FATAL("TLM-2", "DMI not implemented");
        return false;
    }

    /** slave_socket debug transport method (default behavior, can be overridden)
     * @param[in, out] trans Transaction payload object, allocated by initiator, filled here
     * @return The number of bytes read or written
     */
    unsigned int
    slave_dbg_transport(tlm::tlm_generic_payload& trans)
    {
        SC_REPORT_FATAL("TLM-2", "DBG not implemented");
        return false;
    }
};

#endif /* INTSLAVE_H_ */
