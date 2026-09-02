#include "UVLED.h"

void PHY_UvLed_Refresh(void)
{
    //CH_LED_switch(PHY_CH[0].Uvch, PHY_CH[0].Error, PHY_CH[0].Uvon);
    //CH_LED_switch(PHY_CH[1].Uvch, PHY_CH[0].Error, PHY_CH[0].Uvon);
    //CH_LED_switch(PHY_CH[2].Uvch, PHY_CH[0].Error, PHY_CH[0].Uvon);
    //CH_LED_switch(PHY_CH[3].Uvch, PHY_CH[0].Error, PHY_CH[0].Uvon);
}

void PHY_UVLed_Select(u8 ch)
{
    switch(ch)
    {
        case 1:
            PHY_CH[0].Option = 1;
            PHY_CH[1].Option = 0;
            PHY_CH[2].Option = 0;
            PHY_CH[3].Option = 0;
            break;
        
        case 2:
            PHY_CH[0].Option = 0;
            PHY_CH[1].Option = 1;
            PHY_CH[2].Option = 0;
            PHY_CH[3].Option = 0;
            break;
        
        case 3:
            PHY_CH[0].Option = 0;
            PHY_CH[1].Option = 0;
            PHY_CH[2].Option = 1;
            PHY_CH[3].Option = 0;
            break;
        
        case 4:
            PHY_CH[0].Option = 0;
            PHY_CH[1].Option = 0;
            PHY_CH[2].Option = 0;
            PHY_CH[3].Option = 1;
            break;
        
        case 5:
            PHY_CH[0].Option = 1;
            PHY_CH[1].Option = 1;
            PHY_CH[2].Option = 1;
            PHY_CH[3].Option = 1;
            break;
        
        default:
            PHY_CH[0].Option = 0;
            PHY_CH[1].Option = 0;
            PHY_CH[2].Option = 0;
            PHY_CH[3].Option = 0;
            break;
    }

}

