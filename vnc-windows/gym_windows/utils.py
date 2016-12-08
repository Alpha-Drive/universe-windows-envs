from __future__ import print_function
import _winreg
import os
import urllib
import win32gui
import csv
import subprocess
import zipfile
from urlparse import urlparse

import requests
import sys
import six
from six.moves import input
import logging

from constants import *

logger = logging.getLogger()


### Windows Helpers ####

def set_reg_values(reg_path, name_value_tuple):
    for name, value in name_value_tuple:
        # TODO: check for type of each value and turn it into the correct kind (
        # eg string => _winreg.REG_SZ, int => _winreg.REG_DWORD
        set_reg(reg_path, name, value)


def set_reg(reg_path, name, value, kind=_winreg.REG_DWORD):
    try:
        _winreg.CreateKey(_winreg.HKEY_CURRENT_USER, reg_path)
        registry_key = _winreg.OpenKey(_winreg.HKEY_CURRENT_USER, reg_path, 0,
                                       _winreg.KEY_WRITE)
        _winreg.SetValueEx(registry_key, name, 0, kind, value)
        _winreg.CloseKey(registry_key)
        return True
    except WindowsError:
        return False


def get_reg(name, reg_path):
    try:
        registry_key = _winreg.OpenKey(_winreg.HKEY_CURRENT_USER, reg_path, 0,
                                       _winreg.KEY_READ)
        value, regtype = _winreg.QueryValueEx(registry_key, name)
        _winreg.CloseKey(registry_key)
        return value
    except WindowsError:
        return None


def processes_are_running(expected):
    if type(expected) is not list:
        expected = [expected]
    expected = set(expected)
    actual = subprocess.Popen('tasklist.exe /fo csv', stdout=subprocess.PIPE, universal_newlines=True)
    actual = list(csv.DictReader(actual.stdout))
    actual = [p['Image Name'] for p in actual]
    intersection = set(actual).intersection(expected)
    ret = intersection == expected
    return ret


def enum_window_titles():
    def callback(handle, data):
        titles.append(win32gui.GetWindowText(handle))

    titles = []
    win32gui.EnumWindows(callback, None)
    return titles


def download_file(url, path):
    """Good for downloading large files from dropbox as it sets gzip headers and decodes automatically on download"""
    with open(path, "wb") as f:
        logger.info('Downloading %s', url)
        response = requests.get(url, stream=True)
        total_length = response.headers.get('content-length')

        if total_length is None:  # no content length header
            f.write(response.content)
        else:
            dl = 0
            total_length = int(total_length)
            for data in response.iter_content(chunk_size=4096):
                dl += len(data)
                f.write(data)
                done = int(50 * dl / total_length)
                sys.stdout.write("\r[%s%s]" % ('=' * done, ' ' * (50 - done)))
                sys.stdout.flush()


def download_folder(url, dirname):
    """Useful for downloading a folder / zip file from dropbox and unzipping it to path"""
    url_path = urlparse(url).path
    path = os.path.join(dirname, url_path.split('/')[-1])
    path = os.path.splitext(path)[0]
    if os.path.exists(path):
        logger.warn('%s exists, do you want to re-download and overwrite the existing files (y/n)? ', path)
        overwrite = input()
        if 'n' in overwrite.lower():
            logger.info('Using existing %s - Try rerunning and overwriting if you have problems down the line.', path)
            return
    logger.info('Downloading %s to %s', url, path)
    location = urllib.urlretrieve(url)
    location = location[0]
    zip_ref = zipfile.ZipFile(location, 'r')
    logger.info('Unzipping temp file %s to %s', location, path)
    try:
        zip_ref.extractall(dirname)
    except Exception:
        logger.error('You may want to close all programs that may have these files open or delete existing '
              'folders this is trying to overwrite')
        raise
    finally:
        zip_ref.close()
        os.remove(location)

