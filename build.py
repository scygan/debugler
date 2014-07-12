#!/usr/bin/env python
# Copyright (C) 2014 Slawomir Cygan <slawomir.cygan@gmail.com>
#
# Licensed under the Apache License, Version 2.0 (the 'License');
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an 'AS IS' BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


import os
import sys
import subprocess
from optparse import OptionParser
import logging


def getLocalPath():
    return os.path.dirname(os.path.realpath(__file__))

class BaseTarget(object):
    def __init__(self):
        self.deps = []
        pass

    def depend(self, target):
        self.deps.append(target)

    def getBuildTypeStr(self, debug):
        if debug:
            return 'Debug'
        else:
            return 'Release'

    def prepare(self, targetName, debug = False):
        return 0

    def build(self):
        return 0

class CMakeTarget(BaseTarget):
    def __init__(self):
        super(CMakeTarget, self).__init__()


    def prepare(self, targetName, debug, cmakeOpts, toolchain = ''):

        self.path = getLocalPath() + os.sep + 'build' + os.sep + targetName + os.sep + self.getBuildTypeStr(debug)
        
        if not os.path.exists(self.path):
           os.makedirs(self.path)

        cmdLine = cmakeOpts

        if len(toolchain):
            cmdLine += ['-DCMAKE_TOOLCHAIN_FILE=' + toolchain]

        cmdLine += ['-DCMAKE_BUILD_TYPE=' + self.getBuildTypeStr(debug)]

        if not sys.platform.startswith('win'):
            cmdLine += ['-G', 'Eclipse CDT4 - Unix Makefiles']
        else:
            cmdLine += ['-G', 'Eclipse CDT4 - Ninja']
            cmdLine += ['-DCMAKE_MAKE_PROGRAM=' + getLocalPath() + "/tools/ninja.exe"]

        cmdLine += [getLocalPath() + os.sep + "src"]

        logging.debug('Running CMAKE in ' + self.path + ' with args: ' + str(cmdLine) + '...')
        return subprocess.call(['cmake'] + cmdLine, cwd=self.path)


    def build(self, target = 'all'):
        logging.debug('Running make for ' + self.path + '...')
        if not sys.platform.startswith('win'):
            return subprocess.call(['make', '-j', '4', '-C', self.path, target])
        else:
            return subprocess.call([getLocalPath() + "/tools/ninja.exe", '-C', self.path, target], env={"PATH":'c:\\python27'})

class AndroidBuildTarget(CMakeTarget):
    def __init__(self, arch):
        super(AndroidBuildTarget, self).__init__()
        self.arch = arch

    def prepare(self, targetName, debug = False):
        return super(AndroidBuildTarget, self).prepare(targetName, debug, ['-DANDROID_ABI=' + self.arch],  getLocalPath() + '/tools/build/cmake/toolchains/android.toolchain.cmake')

    def build(self):
        return super(AndroidBuildTarget, self).build()


class LinuxBuildTarget(CMakeTarget):
    def __init__(self, arch, cmakeFlags = []):
        super(LinuxBuildTarget, self).__init__()
        self.arch = arch
        self.cmakeFlags = cmakeFlags

    def prepare(self, targetName, debug = False):
        return super(LinuxBuildTarget, self).prepare(targetName, debug, ['-DARCH=' + self.arch] + self.cmakeFlags)

    def build(self):
        return super(LinuxBuildTarget, self).build('package')


class WindowsBuildTarget(BaseTarget):
    def __init__(self, platform, configSuffix):
        super(WindowsBuildTarget, self).__init__()
        self.platform = platform
        self.configSuffix = configSuffix

    def prepare(self, targetName, debug = False):
        self.config = self.getBuildTypeStr(debug) + self.configSuffix
        return 0

    def build(self):
        args = ['debugler.sln', '/p:VisualStudioVersion=11.0', '/m', '/nologo', '/t:Build',  '/p:Configuration=' + self.config + ';platform=' + self.platform]
        logging.debug('Running MSBUILD with args ' + str(args) + '...')
        return subprocess.call([os.getenv('WINDIR') + os.sep + 'Microsoft.NET' + os.sep + 'Framework' + os.sep + 'v4.0.30319' + os.sep + 'MSBuild.exe'] + args, cwd = getLocalPath())



logging.basicConfig(level=logging.INFO)

buildTargets = {}
#Android build targets
buildTargets['android-arm']    = AndroidBuildTarget('armeabi')
buildTargets['android-armv7a'] = AndroidBuildTarget('armeabi-v7a') #not really used
buildTargets['android-x86']    = AndroidBuildTarget('x86')
buildTargets['android-mips']   = AndroidBuildTarget('mips')

buildTargets['android']        = BaseTarget()
buildTargets['android'].depend('android-arm')
buildTargets['android'].depend('android-x86')
buildTargets['android'].depend('android-mips')


if not sys.platform.startswith('win'):
    buildTargets['32']             = LinuxBuildTarget('32')
    buildTargets['64']             = LinuxBuildTarget('64')

    buildTargets['32-dist']         = LinuxBuildTarget('32', ['-DANDROID_PREBUILD=1'])
    buildTargets['32-dist'].depend('android')

    buildTargets['64-dist']         = LinuxBuildTarget('64', ['-DANDROID_PREBUILD=1'])
    buildTargets['64-dist'].depend('android')

else:
    buildTargets['32-dist']         = WindowsBuildTarget('Win32', '-ALL')
    buildTargets['64-dist']         = WindowsBuildTarget('x64',   '-ALL')
    buildTargets['64-dist'].depend('32-dist')
    
    buildTargets['32']              = WindowsBuildTarget('Win32', '')
    buildTargets['64']              = WindowsBuildTarget('x64',   '')



usage = 'usage: %prog [options] [target]'
parser = OptionParser(usage=usage)
parser.set_defaults(build_debug=False)
parser.add_option('-l', '--listTargets', dest='list_targets', action='store_true', help='List avaliable targets')
parser.add_option('-d', '--debug', dest='build_debug', action='store_true', help='Debug build')
parser.add_option('-r', '--release', dest='build_debug', action='store_false', help='Release build')


(options, args) = parser.parse_args()


if options.list_targets:
    for k in buildTargets.keys():
        print k
    exit(0)


def Build(targetName):
    ret = 0
    if targetName not in buildTargets.keys():
        parser.error('Unrecognized target: ' + targetName)
    else:
        target = buildTargets[targetName]

    for dep in target.deps:
        ret = Build(dep)
        if ret != 0:
            return ret

    logging.info('Preparing target ' + targetName)
    ret = target.prepare(targetName, options.build_debug)

    if ret != 0:
        logging.critical('Build prepare failed for target ' + targetName + '. Error code = ' + str(ret) + '.')
        return ret

    ret = target.build()
    if ret != 0:
        logging.critical('Build failed for target ' + targetName + '. Error code = ' + str(ret) + '.')

    return ret

if len(args) > 1: 
    parser.error('Please supply one target to build')
elif len(args) == 1:
    exit(Build(args[0]))
else:
    exit(Build("64-dist"))
