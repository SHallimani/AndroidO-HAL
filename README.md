# AndroidO-HAL

Adding the sample reference example code on basic understand the HIDL design on Android-O

+ When we are making HAL as library make sure the below pre-requisities are taken care
	1/ The ID name defined in HAL module should match the same as the Library name defined in Android.mk
	example:
		< HAL Module >
				struct hw_module_t HAL_MODULE_INFO_SYM = {
					.tag = HARDWARE_MODULE_TAG,
					.module_api_version = DUMMY_MODULE_API_VERSION_1_0,
					.hal_api_version = HARDWARE_HAL_API_VERSION,
					.id = "mydevice"
					.name = "Dummy random HAL",
					.author = "The Android Open Source Project",
					.methods = &dummy_module_methods,
					};
					
		< Library Build >
			Android.mk:
				LOCAL_MODULE := mydevice.default
