//
//  LGWMI.cpp
//  LGWMI
//
//  Copyright © 2020 Le Bao Hiep. All rights reserved.
//

#include "LGWMI.hpp"

bool ADDPR(debugEnabled) = false;
uint32_t ADDPR(debugPrintDelay) = 0;

#define super IOService
OSDefineMetaClassAndStructors(LGWMI, IOService)

IOService *LGWMI::probe(IOService *provider, SInt32 *score) {
    if (!super::probe(provider, score)) {
        return NULL;
    }

    mapDevice = OSDynamicCast(IOACPIPlatformDevice, provider);
    if (!mapDevice) {
        return NULL;
    }

    if (mapDevice->validateObject("WMAB") != kIOReturnSuccess || mapDevice->validateObject("WMBB") != kIOReturnSuccess) {
        return NULL;
    }

    return this;
}

bool LGWMI::start(IOService *provider) {
    if (!provider || !super::start(provider)) {
        SYSLOG("wmi", "failed to start super");
        return false;
    }

    setProperty("Copyright", "Copyright © 2020 Le Bao Hiep. All rights reserved.");

    extern kmod_info_t kmod_info;
    setProperty("LGWMI-Version", kmod_info.version);
#ifdef DEBUG
    setProperty("LGWMI-Build", "Debug");
#else
    setProperty("LGWMI-Build", "Release");
#endif

    registerVSMC();

    registerService();

    return true;
}

int LGWMI::lg_wmab(uint32_t method_id, uint32_t arg1, uint32_t arg2) {
    OSObject *params[3];

    params[0] = OSNumber::withNumber(method_id, 32);
    params[1] = OSNumber::withNumber(arg1, 32);
    params[2] = OSNumber::withNumber(arg2, 32);

    uint32_t val;
    IOReturn ret = mapDevice->evaluateInteger("WMAB", &val, params, 3);
    params[0]->release();
    params[1]->release();
    params[2]->release();

    if (ret != kIOReturnSuccess) {
        DBGLOG("wmi", "lg_wmab failed");
        return -1;
    }

    return val;
}

int LGWMI::lg_wmbb(uint32_t method_id, uint32_t arg1, uint32_t arg2) {
    uint8_t buf[32];
    *(uint32_t *)(buf     ) = method_id;
    *(uint32_t *)(buf + 4 ) = arg1;
    *(uint32_t *)(buf + 16) = arg2;

    OSObject *params[3];

    params[0] = OSNumber::withNumber(0ull, 32); // ignored
    params[1] = OSNumber::withNumber(1ull, 32); // Must be 1 or 2. Does not matter which
    params[2] = OSData::withBytes(buf, 32);

    uint32_t val;
    IOReturn ret = mapDevice->evaluateInteger("WMBB", &val, params, 3);
    params[0]->release();
    params[1]->release();
    params[2]->release();

    if (ret != kIOReturnSuccess) {
        DBGLOG("wmi", "lg_wmbb failed");
        return -1;
    }

    return val;
}

void LGWMI::setBatteryConservativeMode(bool state) {
    if (lg_wmab(WM_BATT_LIMIT, WM_SET, state ? 80 : 100) != 0) {
        SYSLOG("batt", "Failed to %s battery conservative mode", state ? "enable" : "disable");
    } else {
        DBGLOG("batt", "Battery conservative mode is %s", state ? "enabled" : "disabled");
        setProperty("BatteryConservativeMode", state);
    }
}

void LGWMI::setFnLockMode(bool state) {
    if (lg_wmab(WM_FN_LOCK, WM_SET, state) != 0) {
        SYSLOG("batt", "Failed to %s Fn lock", state ? "enable" : "disable");
    } else {
        DBGLOG("batt", "Fn lock is %s", state ? "enabled" : "disabled");
        setProperty("FnLockMode", state);
    }
}

void LGWMI::setUSBChargeMode(bool state) {
    if (lg_wmbb(WMBB_USB_CHARGE, WM_SET, state) != 0) {
        SYSLOG("batt", "Failed to %s USB charge mode", state ? "enable" : "disable");
    } else {
        DBGLOG("batt", "USB charge mode is %s", state ? "enabled" : "disabled");
        setProperty("USBChargeMode", state);
    }
}

void LGWMI::registerVSMC() {
    vsmcNotifier = VirtualSMCAPI::registerHandler(vsmcNotificationHandler, this);

    VirtualSMCAPI::addKey(KeyBDVT, vsmcPlugin.data, VirtualSMCAPI::valueWithFlag(
        false, new BDVT(this),
        SMC_KEY_ATTRIBUTE_READ | SMC_KEY_ATTRIBUTE_WRITE | SMC_KEY_ATTRIBUTE_ATOMIC));
}

bool LGWMI::vsmcNotificationHandler(void *sensors, void *refCon, IOService *vsmc, IONotifier *notifier) {
    if (sensors && vsmc) {
        DBGLOG("wmi", "got vsmc notification");
        auto self = static_cast<LGWMI *>(sensors);
        auto ret = vsmc->callPlatformFunction(VirtualSMCAPI::SubmitPlugin, true, sensors, &self->vsmcPlugin, nullptr, nullptr);
        if (ret == kIOReturnSuccess) {
            DBGLOG("wmi", "Submitted plugin");
            return true;
        } else if (ret != kIOReturnUnsupported) {
            SYSLOG("wmi", "Plugin submission failure %X", ret);
        } else {
            DBGLOG("wmi", "Plugin submission to non vsmc");
        }
    } else {
        SYSLOG("wmi", "Got null vsmc notification");
    }

    return false;
}

EXPORT extern "C" kern_return_t ADDPR(kern_start)(kmod_info_t *, void *) {
    // Report success but actually do not start and let I/O Kit unload us.
    // This works better and increases boot speed in some cases.
    PE_parse_boot_argn("liludelay", &ADDPR(debugPrintDelay), sizeof(ADDPR(debugPrintDelay)));
    ADDPR(debugEnabled) = checkKernelArgument("-vsmcdbg") || checkKernelArgument("-lgwmidbg");
    return KERN_SUCCESS;
}

EXPORT extern "C" kern_return_t ADDPR(kern_stop)(kmod_info_t *, void *) {
    // It is not safe to unload VirtualSMC plugins!
    return KERN_FAILURE;
}
