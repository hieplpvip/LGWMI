//
//  LGWMIUserClient.cpp
//  LGWMI
//
//  Created by Le Bao Hiep on 28/09/2020.
//  Copyright © 2020 Le Bao Hiep. All rights reserved.
//

#include "LGWMIUserClient.hpp"

#define super IOUserClient
OSDefineMetaClassAndStructors(LGWMIUserClient, IOUserClient);

bool LGWMIUserClient::start(IOService *provider) {
    if (!super::start(provider)) {
        return false;
    }

    mDevice = OSDynamicCast(LGWMI, provider);
    if (!mDevice) {
        return false;
    }

    mDevice->retain();

    return true;
}

void LGWMIUserClient::free(void) {
    mDevice->release();
    super::free();
}

IOReturn LGWMIUserClient::clientClose() {
    terminate();
    return kIOReturnSuccess;
}

const IOExternalMethod methodDescs[kNumberOfMethods] = {
    { NULL, (IOMethod) &LGWMIUserClient::methodSet, kIOUCStructIStructO, kIOUCVariableStructureSize, kIOUCVariableStructureSize },
};

IOExternalMethod *LGWMIUserClient::getTargetAndMethodForIndex(IOService **target, UInt32 index) {
    *target = this;
    if (index < kNumberOfMethods) {
        return (IOExternalMethod *) (methodDescs + index);
    }

    return NULL;
}

IOReturn LGWMIUserClient::methodSet(int *which, bool *state, IOByteCount inputSize, IOByteCount *outputSize) {
    if (!which || !state) {
        return kIOReturnBadArgument;
    }

    if (!mDevice) {
        return kIOReturnError;
    }

    switch (*which) {
        case 1: // Fn Lock
            mDevice->setFnLockMode(*state);
            break;
        case 2: // USB Charge
            mDevice->setUSBChargeMode(*state);
            break;
        default:
            return kIOReturnBadArgument;
    }

    return kIOReturnSuccess;
}
