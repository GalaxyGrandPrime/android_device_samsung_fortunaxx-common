/*
 * Copyright 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This contains the module build definitions for the hardware-specific
 * components for this device.
 * As much as possible, those components should be built unconditionally,
 * with device-specific names to avoid collisions, to avoid device-specific
 * bitrot and build breakages. Building a component unconditionally does
 * *not* include it on all devices, so it is safe even with hardware-specific
 * components.
 */

#include <errno.h>
#include <fcntl.h>
#include <string.h>

#define LOG_TAG "PowerHAL"
#include <utils/Log.h>

#include <cutils/properties.h>
#include <hardware/power.h>

enum {
    PROFILE_POWER_SAVE,
    PROFILE_BALANCED,
    PROFILE_HIGH_PERFORMANCE,
    PROFILE_BIAS_POWER,
    PROFILE_BIAS_PERFORMANCE,
};

#define POWER_NR_OF_SUPPORTED_PROFILES 5

#define POWER_PROFILE_PROPERTY  "sys.perf.profile"
#define POWER_SAVE_PROP         "0"
#define BALANCED_PROP           "1"
#define HIGH_PERFORMANCE_PROP   "2"
#define BIAS_POWER_PROP         "3"
#define BIAS_PERFORMANCE_PROP   "4"

/* touchscreen */
#define TS_POWER "/sys/class/input/input3/enabled"

static int current_power_profile = -1;

static void power_init(struct power_module *module __unused)
{
    ALOGI("%s", __func__);
}

static void sysfs_write(char *path, char *s) {
    char buf[80];
    int len;
    int fd = open(path, O_WRONLY);

    if (fd < 0) {
        strerror_r(errno, buf, sizeof(buf));
        ALOGE("Error opening %s: %s\n", path, buf);
        return;
    }

    len = write(fd, s, strlen(s));
    if (len < 0) {
        strerror_r(errno, buf, sizeof(buf));
        ALOGE("Error writing to %s: %s\n", path, buf);
    }

    close(fd);
}

static void power_set_interactive(struct power_module *module, int on)
{
    ALOGD("%s: %s input devices", __func__, on ? "enabling" : "disabling");
    sysfs_write(TS_POWER, on ? "1" : "0");
}

static void set_power_profile(int profile)
{
    if (profile == current_power_profile)
        return;

    switch (profile) {
    case PROFILE_POWER_SAVE:
        property_set(POWER_PROFILE_PROPERTY, POWER_SAVE_PROP);
        break;
    case PROFILE_BALANCED:
        property_set(POWER_PROFILE_PROPERTY, BALANCED_PROP);
        break;
    case PROFILE_HIGH_PERFORMANCE:
        property_set(POWER_PROFILE_PROPERTY, HIGH_PERFORMANCE_PROP);
        break;
    case PROFILE_BIAS_POWER:
        property_set(POWER_PROFILE_PROPERTY, BIAS_POWER_PROP);
        break;
    case PROFILE_BIAS_PERFORMANCE:
        property_set(POWER_PROFILE_PROPERTY, BIAS_PERFORMANCE_PROP);
        break;
    }

    current_power_profile = profile;
}

static void power_hint(struct power_module *module __unused, power_hint_t hint,
                void *data __unused)
{
    if (hint == POWER_HINT_SET_PROFILE)
        set_power_profile(*(int32_t *)data);
}

static struct hw_module_methods_t power_module_methods = {
    .open = NULL,
};

static void set_feature(struct power_module *module __unused,
                feature_t feature, int state)
{
}

static int get_feature(struct power_module *module __unused, feature_t feature)
{
    if (feature == POWER_FEATURE_SUPPORTED_PROFILES)
        return POWER_NR_OF_SUPPORTED_PROFILES;
    return -1;
}

struct power_module HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = POWER_MODULE_API_VERSION_0_3,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = POWER_HARDWARE_MODULE_ID,
        .name = "Samsung Grand Prime PowerHAL",
        .author = "The Android Open Source Project",
        .methods = &power_module_methods,
    },
    .init = power_init,
    .powerHint = power_hint,
    .setInteractive = power_set_interactive,
    .setFeature = set_feature,
    .getFeature = get_feature
};