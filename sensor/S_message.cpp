//
// Created by Sebastien on 21.04.2017.
//

// STL headers
// C headers
// Framework headers
// Library headers
#include "RF24Network_config.h"
// Project headers
// This component's header
#include "S_message.h"

char S_message::buffer[36];

/****************************************************************************/

char* S_message::toString(void)
{
    snprintf(buffer,sizeof(buffer),"measure %2u %2u.%02uV %ulost",
             measure,
             voltage_reading,
             ( voltage_reading & 0xFF ) * 100 / 256,
             lost_packets
    );
    return buffer;
}
