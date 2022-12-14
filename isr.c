#include <xc.h>
#include "main.h"

extern char sec , min;
void __interrupt() isr(void)
{
    static unsigned int count = 0;
    
    if (TMR2IF == 1)
    {
        if (++count == 1250)  // 1 sec time
        {
            count = 0;
            
            //LED1 = !LED1;
            if(sec > 0)
            {
                 sec--;
            }
            else if((sec == 0) && (min > 0))
            {
                min--;
                sec = 59;
            }
        }
        
        TMR2IF = 0;
    }
}
