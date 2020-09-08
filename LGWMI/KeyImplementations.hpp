//
//  KeyImplementations.hpp
//  LGWMI
//
//  Copyright © 2020 Le Bao Hiep. All rights reserved.
//
//  Ambient light sensor support is based on SMCLightSensor

#ifndef KeyImplementations_hpp
#define KeyImplementations_hpp

#include <IOKit/IOService.h>
#include <VirtualSMCSDK/kern_vsmcapi.hpp>

static constexpr SMC_KEY KeyBDVT = SMC_MAKE_IDENTIFIER('B','D','V','T');

class BDVT : public VirtualSMCValue {
    IOService *dst {nullptr};

protected:
    SMC_RESULT update(const SMC_DATA *src) override;

public:
    BDVT(IOService *dst) : dst(dst) {}
};

#endif /* KeyImplementations_hpp */
