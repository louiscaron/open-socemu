#include "Aic.h"
#include "utils.h"

/// Used to select debugging (0 = OFF, 1 = ON)
#define DEBUG_AIC 0

// Used to make direct register access
uint32_t* reg_aic;

// using this namespace to simplify streaming
using namespace std;

/** Register read function
 * @param[in] offset Offset of the register to read
 * @return The value read
 */
uint32_t
Aic::reg_rd(uint32_t offset)
{
    // retrieve the required parameters
    uint32_t index = offset/4;

    // sanity check
    assert(index < REG_AIC_COUNT);

    return m_reg[index];
}

/** Register write function
 * @param[in] offset Offset of the register to read
 */
void
Aic::reg_wr(uint32_t offset, uint32_t value)
{
    // retrieve the required parameters
    uint32_t index = offset/4;

    // sanity check
    assert(index < REG_AIC_COUNT);

    switch (index)
    {
    case AIC_IECR_INDEX:
        // when writing to this register
        aic_imr_set(aic_imr_get() | value);
        break;
    case AIC_IDCR_INDEX:
        aic_imr_set(aic_imr_get() & (~value));
        break;
    case AIC_IMR_INDEX:
        // read only register
        break;
    case AIC_FFER_INDEX:
        aic_ffsr_set(aic_ffsr_get() | value);
        break;
    case AIC_FFDR_INDEX:
        aic_ffsr_set(aic_ffsr_get() & (~value));
        break;
    case AIC_FFSR_INDEX:
        // read only register
        break;
    default:
        m_reg[index] = value;
        break;
    }

    // check if interrupt configuration has changed
    check_int();
}

void
Aic::check_int()
{
    // find the highest interrupt pending
    unsigned i;
    uint32_t mask;

    mask = aic_ipr_get() & aic_imr_get();

    // by default clear all IRQ and FIQ
    aic_cisr_set(0);

    // generate the FIQ
    for (i = 0; i < 32; i++)
    {
        // check if FIQ is pending (special case for PID0 = FIQ)
        if (((i == 0) && (mask & 1)) || (aic_ffsr_get() & mask & (1 << i)))
        {
            // the current interrupt source number
            aic_isr_set(i);
            // FIQ pending
            aic_cisr_set(aic_cisr_get() | NFIQ_BIT);
            // FIQ vector
            aic_fvr_set(aic_svr_get(0));

            break;
        }
    }
    if (aic_cisr_nfiq_getf())
    {
        // set the FIQ
        this->fiq.set();
    }
    else
    {
        // clear the FIQ
        this->fiq.clear();
    }

    // generate the IRQ
    for (i = 1; i < 32; i++)
    {
        // check if IRQ is pending (special case for PID0 = FIQ)
        if ((~aic_ffsr_get()) & mask & (1 << i))
        {
            // update the ISR except if there is already an FIQ pending
            if (aic_cisr_nfiq_getf() == 0)
            {
                // the current interrupt source number
                aic_isr_set(i);
            }
            // IRQ pending
            aic_cisr_set(aic_cisr_get() | NIRQ_BIT);
            // FIQ vector
            aic_ivr_set(m_reg[AIC_SVR_INDEX+i]);

            break;
        }
    }
    if (aic_cisr_nirq_getf())
    {
        // set the IRQ
        this->irq.set();
    }
    else
    {
        // clear the IRQ
        this->irq.clear();
    }
}

void
Aic::interrupt_set(void* opaque)
{
    int id = (int)opaque;

    // set the register value
    aic_ipr_set(aic_ipr_get() | (1 << id));

    // check if interrupt configuration has changed
    check_int();
}

void
Aic::interrupt_clr(void* opaque)
{
    int id = (int)opaque;

    // clear the register value
    aic_ipr_set(aic_ipr_get() & (~(1 << id)));

    // check if interrupt configuration has changed
    check_int();
}
