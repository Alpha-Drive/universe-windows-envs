from __future__ import print_function

import csv
import glob
import hashlib
import sys
import os
import urllib
import zipfile

import shutil

# This file is self-contained so as to allow use outside of this context.
import subprocess

try:
    input = raw_input
except NameError:
    pass

GTAV_PROCESS_NAME = 'GTA5.exe'

def detect_binary_changes(gtav_dir):
    md5_paths = [
        ('e40919966756d7bcff79a9a98e4b9522', os.path.join(gtav_dir, 'GTA5.exe')),
        ('38e62da773629feb9369a2bd9cad8a53', os.path.join(gtav_dir, 'GTAVLauncher.exe')),
        ('e2e9e2ab49feb736381a7e34cc3e264b', get_update_rpf_path(gtav_dir)),
        ('7129fb28c6a679f36b9c9461e1cadf6a', get_social_club_path(gtav_dir))
    ]

    for md5_path in md5_paths:
        file_path = md5_path[1]
        expected_md5 = md5_path[0]
        actual_md5 = hash_large_file(file_path)
        if expected_md5 == actual_md5:
            print('\t verified', file_path)
        else:
            print("""
------------------------------------------------------------------------------------------------------------------------
FILE CHANGE DETECTED - %s
------------------------------------------------------------------------------------------------------------------------
""" % file_path)
            return True
    return False


def gtav_is_running():
    p_tasklist = subprocess.Popen('tasklist.exe /fo csv',
                                  stdout=subprocess.PIPE,
                                  universal_newlines=True)

    pythons_tasklist = []
    tasks = list(csv.DictReader(p_tasklist.stdout))
    for p in tasks:
        if p['Image Name'] == GTAV_PROCESS_NAME:
            pythons_tasklist.append(p)
    if len(pythons_tasklist) > 0:
        return True
    else:
        return False


def get_social_club_path(gtav_dir):
    return glob.glob(os.path.join(gtav_dir, 'Installers', 'Social-Club*.exe'))[0]


def get_update_rpf_path(gtav_dir):
    return os.path.join(gtav_dir, 'update', 'update.rpf')


def hash_large_file(file_path):
    """Avoid reading entire file into memory by setting block size to a reasonable value"""
    BLOCKSIZE = 2 ** 20
    hasher = hashlib.md5()
    with open(file_path, 'rb') as afile:
        buf = afile.read(BLOCKSIZE)
        while len(buf) > 0:
            hasher.update(buf)
            buf = afile.read(BLOCKSIZE)
    return hasher.hexdigest()


def restore_game_files(gtav_dir):
    while gtav_is_running():
        # TODO: Figure out nice way of closing GTAV programmatically (killing causes recovery splash next startup)
        print(GTAV_PROCESS_NAME + ' is open, please close it and press any key to continue...')
        input()
    # TODO: reuse existing download if present
    print('Downloading GTAV binaries - these are 665MB, so could take a while...')
    location = urllib.urlretrieve('https://www.dropbox.com/sh/ymt2ja4st2mpvyd/AADZBlzxsxiEwQm15Y15nC1wa?dl=1')
    location = location[0]
    print('Extracting GTAV binaries from', location, 'to', gtav_dir)
    zip_ref = zipfile.ZipFile(location, 'r')
    zip_ref.extractall(gtav_dir)
    zip_ref.close()
    shutil.move(os.path.join(gtav_dir, 'update.rpf'), get_update_rpf_path(gtav_dir))
    shutil.move(os.path.join(gtav_dir, 'Social-Club-v1.1.9.6-Setup.exe'), get_social_club_path(gtav_dir))
    print('Finished extracting GTAV files, deleting temp file:', location)
    os.remove(location)


def enforce_version(gtav_dir):
    print('Verifying GTAV version...')
    changes_detected = detect_binary_changes(gtav_dir)
    if changes_detected:
        print('Detected change in GTAV files. This likely means the game was updated which breaks ScriptHookV and can cause unwanted changes to environment dynamics.')
        resp = input('Do you want to restore to a known working version of the game? \nNote that this WILL NOT re-download the entire game or circumvent DRM - it will just roll back the updated portion of the game (y/n)? ')
        if 'y' in resp.lower():
            restore_game_files(gtav_dir)
        else:
            print('Not restoring game files.')
    else:
        print('GTAV is at the correct version')

if __name__ == '__main__':
    if 'TEST_RESTORE_GTA' in os.environ:
        restore_game_files(os.environ['GTAV_DIR'])
    sys.exit(enforce_version(os.environ['GTAV_DIR']))
