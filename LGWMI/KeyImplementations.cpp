//
//  KeyImplementations.cpp
//  LGWMI
//
//  Copyright © 2020 Le Bao Hiep. All rights reserved.
//

#include "KeyImplementations.hpp"
#include "LGWMI.hpp"

SMC_RESULT BDVT::update(const SMC_DATA *src)  {
    bool state = false;
    lilu_os_memcpy(&state, src, size);

    // BDVT is 00 when battery health is enabled and 01 when disabled
    state = !state;

    LGWMI *drv = OSDynamicCast(LGWMI, dst);
    drv->setBatteryConservativeMode(state);

    lilu_os_memcpy(data, src, size);
    return SmcSuccess;
}
