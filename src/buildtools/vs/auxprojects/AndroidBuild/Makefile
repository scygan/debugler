all: $(OUT)\.dir $(OUT)\libs\$(ABI)\libdglwrapper.so 

$(OUT)\.dir:
	mkdir $(OUT)
	echo dir > $(OUT)\.dir

$(OUT)\libs\$(ABI)\libdglwrapper.so: $(OUT)\Makefile
	cd $(OUT)
	set PATH=%PATH%;c:\python27
	nmake

$(OUT)\bin\$(ABI)\dglloader: $(OUT)\Makefile $(OUT)\libs\$(ABI)\libdglwrapper.so
	cd $(OUT)
	set PATH=%PATH%;c:\python27
	nmake

$(OUT)\Makefile:
	cd $(OUT)
	cmake -DCMAKE_TOOLCHAIN_FILE=..\..\..\src\buildtools\toolchains\android.toolchain.cmake -DCMAKE_BUILD_TYPE=$(TYPE) -DANDROID_ABI=$(ABI) ..\..\..\src\ -G "Eclipse CDT4 - NMake Makefiles"
