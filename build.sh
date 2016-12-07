#!/bin/sh
#set -eu # Bashism, does not work with default shell on Ubuntu 12.04

install_apk=false
run_apk=false
sign_apk=false
build_release=false

if [ "$#" -gt 0 -a "$1" = "-s" ]; then
	shift
	sign_apk=true
fi

if [ "$#" -gt 0 -a "$1" = "-i" ]; then
	shift
	install_apk=true
fi

if [ "$#" -gt 0 -a "$1" = "-r" ]; then
	shift
	install_apk=true
	run_apk=true
fi

if [ "$#" -gt 0 -a "$1" = "release" ]; then
	shift
	build_release=true
fi

[ -e project/local.properties ] || {
	android update project -p project || exit 1
	rm -f project/src/Globals.java
}
# Set here your own NDK path if needed
# export PATH=$PATH:~/src/endless_space/android-ndk-r7
NDKBUILDPATH=$PATH
export `grep "AppFullName=" AndroidAppSettings.cfg`
if ( grep "package $AppFullName;" project/src/Globals.java > /dev/null 2>&1 && \
		[ "`readlink AndroidAppSettings.cfg`" -ot "project/src/Globals.java" ] && \
		[ -z "`find project/java/* project/AndroidManifestTemplate.xml -cnewer project/src/Globals.java`" ] ) ; then true ; else
	./changeAppSettings.sh -a
	sleep 1
	touch project/src/Globals.java
fi

if $build_release ; then
	sed -i 's/android:debuggable="true"/android:debuggable="false"/g' project/AndroidManifest.xml
else
	sed -i 's/android:debuggable="false"/android:debuggable="true"/g' project/AndroidManifest.xml
fi

MYARCH=linux-x86
NCPU=4
if uname -s | grep -i "linux" > /dev/null ; then
	MYARCH=linux-x86
	NCPU=`cat /proc/cpuinfo | grep -c -i processor`
fi
if uname -s | grep -i "darwin" > /dev/null ; then
	MYARCH=darwin-x86
fi
if uname -s | grep -i "windows" > /dev/null ; then
	MYARCH=windows-x86
fi
grep "64.bit" "`which ndk-build | sed 's@/ndk-build@@'`/RELEASE.TXT" >/dev/null 2>&1 && MYARCH="${MYARCH}_64"

rm -r -f project/bin/* # New Android SDK introduced some lame-ass optimizations to the build system which we should take care about
[ -x project/jni/application/src/AndroidPreBuild.sh ] && {
	cd project/jni/application/src
	./AndroidPreBuild.sh || { echo "AndroidPreBuild.sh returned with error" ; exit 1 ; }
	cd ../../../..
}

cd project && env PATH=$NDKBUILDPATH BUILD_NUM_CPUS=$NCPU nice -n19 ndk-build -j$NCPU V=1 && \
	{	grep "CustomBuildScript=y" ../AndroidAppSettings.cfg > /dev/null && \
		echo Stripping libapplication-armeabi.so by hand && \
		rm obj/local/armeabi/libapplication.so && \
		cp jni/application/src/libapplication-armeabi.so obj/local/armeabi/libapplication.so && \
		cp jni/application/src/libapplication-armeabi.so libs/armeabi/libapplication.so && \
		`which ndk-build | sed 's@/ndk-build@@'`/toolchains/arm-linux-androideabi-4.6/prebuilt/$MYARCH/bin/arm-linux-androideabi-strip --strip-unneeded libs/armeabi/libapplication.so \
		|| true ; } && \
	{	grep "CustomBuildScript=y" ../AndroidAppSettings.cfg > /dev/null && \
		grep "MultiABI=" ../AndroidAppSettings.cfg | grep "y\\|all\\|armeabi-v7a" > /dev/null && \
		echo Stripping libapplication-armeabi-v7a.so by hand && \
		rm obj/local/armeabi-v7a/libapplication.so && \
		cp jni/application/src/libapplication-armeabi-v7a.so obj/local/armeabi-v7a/libapplication.so && \
		cp jni/application/src/libapplication-armeabi-v7a.so libs/armeabi-v7a/libapplication.so && \
		`which ndk-build | sed 's@/ndk-build@@'`/toolchains/arm-linux-androideabi-4.6/prebuilt/$MYARCH/bin/arm-linux-androideabi-strip --strip-unneeded libs/armeabi-v7a/libapplication.so \
		|| true ; } && \
	{	grep "CustomBuildScript=y" ../AndroidAppSettings.cfg > /dev/null && \
		grep "MultiABI=" ../AndroidAppSettings.cfg | grep "armeabi-v7a-hard" > /dev/null && \
		echo Stripping libapplication-armeabi-v7a-hard.so by hand && \
		rm obj/local/armeabi-v7a-hard/libapplication.so && \
		cp jni/application/src/libapplication-armeabi-v7a-hard.so obj/local/armeabi-v7a-hard/libapplication.so && \
		cp jni/application/src/libapplication-armeabi-v7a-hard.so libs/armeabi-v7a/libapplication.so && \
		`which ndk-build | sed 's@/ndk-build@@'`/toolchains/arm-linux-androideabi-4.6/prebuilt/$MYARCH/bin/arm-linux-androideabi-strip --strip-unneeded libs/armeabi-v7a/libapplication.so \
		|| true ; } && \
	{	grep "CustomBuildScript=y" ../AndroidAppSettings.cfg > /dev/null && \
		grep "MultiABI=" ../AndroidAppSettings.cfg | grep "all\\|mips" > /dev/null && \
		echo Stripping libapplication-mips.so by hand && \
		rm obj/local/mips/libapplication.so && \
		cp jni/application/src/libapplication-mips.so obj/local/mips/libapplication.so && \
		cp jni/application/src/libapplication-mips.so libs/mips/libapplication.so && \
		`which ndk-build | sed 's@/ndk-build@@'`/toolchains/mipsel-linux-android-4.6/prebuilt/$MYARCH/bin/mipsel-linux-android-strip --strip-unneeded libs/mips/libapplication.so \
		|| true ; } && \
	{	grep "CustomBuildScript=y" ../AndroidAppSettings.cfg > /dev/null && \
		grep "MultiABI=" ../AndroidAppSettings.cfg | grep "all\\|x86" > /dev/null && \
		echo Stripping libapplication-x86.so by hand && \
		rm obj/local/x86/libapplication.so && \
		cp jni/application/src/libapplication-x86.so obj/local/x86/libapplication.so && \
		cp jni/application/src/libapplication-x86.so libs/x86/libapplication.so && \
		`which ndk-build | sed 's@/ndk-build@@'`/toolchains/x86-4.6/prebuilt/$MYARCH/bin/i686-linux-android-strip --strip-unneeded libs/x86/libapplication.so \
		|| true ; } && \
	cd .. && ./copyAssets.sh && cd project && \
	{	if $build_release ; then \
			ant release || exit 1 ; \
			if $sign_apk; then
				jarsigner -verbose -keystore ~/.android/debug.keystore -storepass android -sigalg MD5withRSA -digestalg SHA1 bin/MainActivity-release-unsigned.apk androiddebugkey || exit 1 ; \
				zipalign 4 bin/MainActivity-release-unsigned.apk bin/MainActivity-debug.apk ; \
			fi
		else \
			ant -v -d debug ; \
		fi ; } && \
	{	if $sign_apk; then cd .. && ./sign.sh && cd project ; else true ; fi ; } && \
	$install_apk && [ -n "`adb devices | tail -n +2`" ] && \
	{	cd bin && adb install -r MainActivity-debug.apk | grep 'Failure' && \
		adb uninstall `grep AppFullName ../../AndroidAppSettings.cfg | sed 's/.*=//'` && adb install -r MainActivity-debug.apk ; true ; } && \
	$run_apk && { \
		ActivityName="`grep AppFullName ../../AndroidAppSettings.cfg | sed 's/.*=//'`/.MainActivity" ; \
		RUN_APK="adb shell am start -n $ActivityName" ; \
		echo "Running $ActivityName on the USB-connected device:" ; \
		echo "$RUN_APK" ; \
		eval $RUN_APK ; \
	}
