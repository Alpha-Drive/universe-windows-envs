#!/usr/bin/python
from __future__ import print_function
import argparse
import json
import logging
import os
import subprocess
import sys
import csv
import urllib
import zipfile
from sys import platform as _platform
import time
import shutil
from datetime import datetime
from enforce_version import enforce_version

# TODO: Rearrange directories for a proper import
CURR_DIR = os.path.dirname(os.path.realpath(__file__))
GTAV_DIR = os.environ['GTAV_DIR']
UNIVERSE_WINDOWS_ENVS_DIR = os.environ['UNIVERSE_WINDOWS_ENVS_DIR']
SAVED_GAMES_LOCATION = os.path.expanduser('~\\Documents\\Rockstar Games\\GTA V\\Profiles\\')
sys.path.append(os.path.join(os.path.abspath(os.path.join(CURR_DIR, os.pardir)), 'vnc-windows'))
import gym_windows


logger = logging.getLogger()


def get_saved_games_profile_folders():
    profiles = os.listdir(SAVED_GAMES_LOCATION)
    if len(profiles) > 1:
        logger.info('More than one GTAV settings profile found, replacing saved games of all profiles')
    elif len(profiles) == 0:
        raise Exception('No GTAV settings profile found to load saved games into. Aborting')
    return [SAVED_GAMES_LOCATION + profile for profile in profiles]


def download_paths_xml():
    gtav_dir = os.environ['GTAV_DIR']
    logger.info('Downloading paths.xml file to %s - file is 150MB so this may take a while', gtav_dir)
    gym_windows.download_file('https://www.dropbox.com/s/6qc900o3a574yxc/paths.xml?dl=1', os.path.join(gtav_dir, 'paths.xml'))


def setup():
    download_paths_xml()
    replace_saved_games()
    enforce_version(GTAV_DIR)
    copy_scripthook()


def copy_scripthook():
    bin_dir = os.path.join(UNIVERSE_WINDOWS_ENVS_DIR, 'vnc-gtav', 'GTAVScriptHookProxy', 'bin')
    for filename in ['dinput8.dll', 'GTAVScriptHookProxy.asi', 'ScriptHookV.dev', 'ScriptHookV.dll', 'x360ce_x64.exe']:
        dest_path = os.path.join(GTAV_DIR, filename)
        if os.path.isfile(dest_path):
            time_str = datetime.now().strftime('%Y%m%d_%H%M_%S')
            backup_path = '%s.%s.%s' % (dest_path, time_str, 'backup')
            logger.info('Backing up %s to %s', dest_path, backup_path)
            shutil.move(dest_path, backup_path)
        src_path = os.path.join(bin_dir, filename)
        logger.info('Copying %s to %s', src_path, dest_path)
        shutil.copyfile(src_path, dest_path)


def replace_saved_games():
    saved_games_profile_folders = get_saved_games_profile_folders()
    location = urllib.urlretrieve('https://www.dropbox.com/sh/k1osqcufsubo754/AADCeXM4I1iYRz19bdO12pOba?dl=1')
    location = location[0]
    zip_ref = zipfile.ZipFile(location, 'r')
    backup_saved_games()
    for saved_games_profile_folder in saved_games_profile_folders:
        logger.info('Replacing saved games in %s', saved_games_profile_folder)
        zip_ref.extractall(saved_games_profile_folder)
    zip_ref.close()


def backup_saved_games():
    time_str = datetime.now().strftime('%Y%m%d_%H%M_%S')
    backup_location = os.path.expanduser('~\\Documents\\GTAV_saved_games_backup_' + time_str)
    logger.info('Backing up saved games in %s to %s', SAVED_GAMES_LOCATION, backup_location)
    shutil.copytree(SAVED_GAMES_LOCATION, backup_location)


def main():
    parser = argparse.ArgumentParser(description=None)
    args = parser.parse_args()
    logging.basicConfig()
    logger.setLevel(logging.INFO)
    setup()
    logger.info('Installation complete')


if __name__ == '__main__':
    if 'SCRIPTHOOK_COPY_ONLY' in os.environ:
        copy_scripthook()
    else:
        sys.exit(main())
