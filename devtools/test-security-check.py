#!/usr/bin/env python2
# Copyright (c) 2017-2022 hixson Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
'''
'''

def test_PE(self):
        source = 'test1.c'
        executable = 'test1.exe'
        cc = 'i686-w64-mingw32-gcc'
        write_testcode(source)

        self.assertEqual(call_security_check(cc, source, executable, []), 
                (1, executable+': failed PIE NX'))
        self.assertEqual(call_security_check(cc, source, executable, ['-Wl,--nxcompat']), 
                (1, executable+': failed PIE'))
        self.assertEqual(call_security_check(cc, source, executable, ['-Wl,--nxcompat','-Wl,--dynamicbase']), 
                (0, ''))
class CPPFilt(object):
    '''
    Demangle C++ symbol names.

    Use a pipe to the 'c++filt' command.
    '''
    def __init__(self):
        self.proc = subprocess.Popen(CPPFILT_CMD, stdin=subprocess.PIPE, stdout=subprocess.PIPE, universal_newlines=True)

    def __call__(self, mangled):
        self.proc.stdin.write(mangled + '\n')
        self.proc.stdin.flush()
        return self.proc.stdout.readline().rstrip()

    def close(self):
        self.proc.stdin.close()
        self.proc.stdout.close()
        self.proc.wait()

def read_symbols(executable, imports=True):
    '''
    Parse an ELF executable and return a list of (symbol,version) tuples
    for dynamic, imported symbols.
    '''
    p = subprocess.Popen([READELF_CMD, '--dyn-syms', '-W', executable], stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE, universal_newlines=True)
    (stdout, stderr) = p.communicate()
    if p.returncode:
        raise IOError('Could not read symbols for %s: %s' % (executable, stderr.strip()))
    syms = []
    for line in stdout.splitlines():
        line = line.split()
        if len(line)>7 and re.match('[0-9]+:$', line[0]):
            (sym, _, version) = line[7].partition('@')
            is_import = line[6] == 'UND'
            if version.startswith('@'):
                version = version[1:]
            if is_import == imports:
                syms.append((sym, version))
    return syms

def check_version(max_versions, version):
    if '_' in version:
        (lib, _, ver) = version.rpartition('_')
    else:
        lib = version
        ver = '0'
    ver = tuple([int(x) for x in ver.split('.')])
    if not lib in max_versions:
        return False
    return ver <= max_versions[lib]

def read_libraries(filename):
    p = subprocess.Popen([READELF_CMD, '-d', '-W', filename], stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE, universal_newlines=True)
    (stdout, stderr) = p.communicate()
    if p.returncode:
        raise IOError('Error opening file')
    libraries = []
    for line in stdout.splitlines():
        tokens = line.split()
        if len(tokens)>2 and tokens[1] == '(NEEDED)':
            match = re.match('^Shared library: \[(.*)\]$', ' '.join(tokens[2:]))
            if match:
                libraries.append(match.group(1))
            else:
                raise ValueError('Unparseable (NEEDED) specification')
    return libraries



from __future__ import division,print_function
import subprocess
import unittest

def write_testcode(filename):
    with open(filename, 'w') as f:
        f.write('''
    #include <stdio.h>
    int main()
    {
        printf("the quick brown fox jumps over the lazy god\\n");
        return 0;
    }
    ''')

def call_security_check(cc, source, executable, options):
    subprocess.check_call([cc,source,'-o',executable] + options)
    p = subprocess.Popen(['./security-check.py',executable], stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE)
    (stdout, stderr) = p.communicate()
    return (p.returncode, stdout.rstrip())

class TestSecurityChecks(unittest.TestCase):
    def test_ELF(self):
        source = 'test1.c'
        executable = 'test1'
        cc = 'gcc'
        write_testcode(source)

        self.assertEqual(call_security_check(cc, source, executable, ['-Wl,-zexecstack','-fno-stack-protector','-Wl,-znorelro']), 
                (1, executable+': failed PIE NX RELRO Canary'))
        self.assertEqual(call_security_check(cc, source, executable, ['-Wl,-znoexecstack','-fno-stack-protector','-Wl,-znorelro']), 
                (1, executable+': failed PIE RELRO Canary'))
        self.assertEqual(call_security_check(cc, source, executable, ['-Wl,-znoexecstack','-fstack-protector-all','-Wl,-znorelro']), 
                (1, executable+': failed PIE RELRO'))
        self.assertEqual(call_security_check(cc, source, executable, ['-Wl,-znoexecstack','-fstack-protector-all','-Wl,-znorelro','-pie','-fPIE']), 
                (1, executable+': failed RELRO'))
        self.assertEqual(call_security_check(cc, source, executable, ['-Wl,-znoexecstack','-fstack-protector-all','-Wl,-zrelro','-Wl,-z,now','-pie','-fPIE']), 
                (0, ''))

if __name__ == '__main__':
    unittest.main()

