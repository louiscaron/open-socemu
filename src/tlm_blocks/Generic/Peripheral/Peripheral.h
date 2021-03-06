#ifndef PERIPHERAL_H_
#define PERIPHERAL_H_

// derived class
#include "Generic/BusSlave/BusSlave.h"

#define PERIPHERAL_DBG(...)                                                 \
do {                                                                        \
    if (this->m_debug) {                                                    \
        TLM_DBG(__VA_ARGS__);                                               \
    }                                                                       \
} while (0)

template <int REG_COUNT>
struct Peripheral : BusSlave
{
    /// Constructor
    Peripheral(sc_core::sc_module_name name) 
    : BusSlave(name, m_reg, sizeof(m_reg))
    , m_debug(false)
    {
        // clear all registers
        memset(m_reg, 0, sizeof(m_reg));
    }
    
    /// Configure the debug mode
    void
    set_debug(bool debug)
    {
        m_debug = debug;
    }

protected:
    /// Specific blocking transport method
    virtual void
    slave_b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay)
    {
        TLM_WORD_SANITY(trans);

        // retrieve the required parameters
        uint32_t* ptr = reinterpret_cast<uint32_t*>(trans.get_data_ptr());

        // sanity check
        #if BUSSLAVE_DEBUG_LEVEL
        assert(m_free);
        #endif

        // mark as busy
        #if BUSSLAVE_DEBUG_LEVEL
        m_free = false;
        #endif

        if (trans.get_command() == tlm::TLM_READ_COMMAND)
        {
            *ptr = this->reg_rd(trans.get_address());
            PERIPHERAL_DBG("RD[%X] <= %X", (uint32_t)trans.get_address(), *ptr);
        }
        else
        {
            PERIPHERAL_DBG("WR[%X] <= %X", (uint32_t)trans.get_address(), *ptr);
            this->reg_wr(trans.get_address(), *ptr);
        }

        // there was no error in the processing
        trans.set_response_status(tlm::TLM_OK_RESPONSE);

        // mark as free
        #if BUSSLAVE_DEBUG_LEVEL
        m_free = true;
        #endif

        return;
    }

    /** Register read function
     * @param[in] offset Offset of the register to read
     * @return The value read
     */
    virtual uint32_t
    reg_rd(uint32_t offset)
    {
        uint32_t result;
        // retrieve the required parameters
        uint32_t index = offset/4;

        // sanity check
        assert(index < REG_COUNT);

        // internal delay
        this->delay();

        switch (index)
        {
        default:
            // read the register value
            result = m_reg[index];
            break;
        }

        return result;
    }

    /** Register write function
     * @param[in] offset Offset of the register to read
     * @param[in] offset Value to write in the register
     */
    virtual void
    reg_wr(uint32_t offset, uint32_t value)
    {
        // retrieve the required parameters
        uint32_t index = offset/4;

        // sanity check
        assert(index < REG_COUNT);

        // internal delay
        this->delay();

        switch (index)
        {
        default:
            m_reg[index] = value;
            break;
        }
    }

    /// Registers content
    uint32_t m_reg[REG_COUNT];
    
    /// Debug mode
    bool m_debug;
};

#endif /*PERIPHERAL_H_*/
