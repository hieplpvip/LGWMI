//
//  LGWMIUserClient.hpp
//  LGWMI
//
//  Created by Le Bao Hiep on 28/09/2020.
//  Copyright © 2020 Le Bao Hiep. All rights reserved.
//

#ifndef LGWMIUserClient_hpp
#define LGWMIUserClient_hpp

#include <IOKit/IOUserClient.h>
#include "LGWMI.hpp"

#define kNumberOfMethods 1

class LGWMIUserClient : public IOUserClient {
    OSDeclareDefaultStructors(LGWMIUserClient);

private:
    LGWMI *mDevice;

public:
    virtual bool start(IOService *provider) override;
    virtual void free() override;

    virtual IOReturn clientClose() override;
    virtual IOExternalMethod *getTargetAndMethodForIndex(IOService **targetP, UInt32 index) override;

    IOReturn methodSet(int *which, bool *state, IOByteCount inputSize, IOByteCount *outputSize);
};

#endif /* LGWMIUserClient_hpp */
