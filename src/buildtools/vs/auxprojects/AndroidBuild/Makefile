.PHONY: all force
all: $(OUT)\.dir $(OUT)\bin\$(ABI)\dglloader $(OUT)\libs\$(ABI)\libdglwrapper.so

$(OUT)\.dir:
	mkdir $(OUT)
	echo dir > $(OUT)\.dir

$(OUT)\libs\$(ABI)\libdglwrapper.so: $(OUT)\build.ninja force
	cd $(OUT)
	set PATH=%PATH%;c:\python27
	..\..\..\tools\ninja.exe

$(OUT)\bin\$(ABI)\dglloader: $(OUT)\build.ninja force
	cd $(OUT)
	set PATH=%PATH%;c:\python27
	..\..\..\tools\ninja.exe

$(OUT)\build.ninja:
	cd $(OUT)
	cmake -DCMAKE_TOOLCHAIN_FILE=..\..\..\src\buildtools\toolchains\android.toolchain.cmake -DCMAKE_BUILD_TYPE=$(TYPE) -DANDROID_ABI=$(ABI) -DCMAKE_MAKE_PROGRAM=..\..\..\tools\ninja.exe -G "Ninja" ..\..\..\src\ 

force: