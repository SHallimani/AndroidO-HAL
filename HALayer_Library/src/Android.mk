###############################################################################
# Sample Reference Example code for understanding the Android HAL service
###############################################################################
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := android.hardware.sterminal.example@v1.0
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_SRC_FILES := sterminal.c
LOCAL_C_INCLUDES := hardware/libhardware/include/
LOCAL_SHARED_LIBRARIES := liblog
LOCAL_CFLAGS += -Wall -Wextra -Werror -Wno-unused-parameter

include $(BUILD_SHARED_LIBRARY)

# Build HAL command line test suite utility.
include $(CLEAR_VARS)

LOCAL_SRC_FILES := test/sterminal_test_tool.c
LOCAL_MODULE := sterminal_test_tool
LOCAL_CFLAGS := -Wall -Wno-unused-parameter -Werror
LOCAL_SHARED_LIBRARIES := libcutils libhardware liblog

include $(BUILD_EXECUTABLE)