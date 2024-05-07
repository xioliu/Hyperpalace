#include <stdint.h>
#include "aarch64.h"

void psw_save(uint64_t *psw)
{
    uint64_t psw_local;

    do{
        psw_local = raw_read_daif();
    }while(0);
    *psw = psw_local;
}

void psw_restore(uint64_t *psw)
{
    uint64_t psw_local = *psw;
    
    do
    {
        raw_write_daif(psw_local);
    } while (0);
}