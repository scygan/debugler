all: $(OUT)\.dir $(OUT)\android-arm\libs\armeabi-v7a\libdglwrapper.so 

$(OUT)\.dir:
	mkdir $(OUT)
	echo dir > $(OUT)\.dir

$(OUT)\android-arm\libs\armeabi-v7a\libdglwrapper.so: $(OUT)\Makefile
	cd $(OUT)
	set PATH=%PATH%;c:\python27
	nmake

$(OUT)\android-arm\bin\dglloader: $(OUT)\Makefile $(OUT)\android-arm\libs\armeabi-v7a\libdglwrapper.so
	cd $(OUT)
	set PATH=%PATH%;c:\python27
	nmake

$(OUT)\Makefile:
	cd $(OUT)
	cmake -DCMAKE_TOOLCHAIN_FILE=..\..\src\buildtools\toolchains\android.toolchain.cmake -DCMAKE_BUILD_TYPE=$(TYPE) -DANDROID_ABI=armeabi ..\..\src\ -G "Eclipse CDT4 - NMake Makefiles"
