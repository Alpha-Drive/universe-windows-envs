from __future__ import print_function
import subprocess
import urllib

from utils import set_reg_values, enum_window_titles
import time
import win32gui
import logging

logger = logging.getLogger()


TIGHTVNC_DIR = r"C:/Program Files/TightVNC"


def start_tvnserver_command():
    return r'"{TIGHTVNC_DIR}/tvnserver.exe" -run'.format(TIGHTVNC_DIR=TIGHTVNC_DIR)


def configure_tvnserver_command(window_name):
    while True:
        count, quoted_window_name = get_window_count(window_name)
        if count > 1:
            logger.error('Multiple occurrences of target VNC window %s '
                         '-- Please close folders or other windows with the same name as the game', quoted_window_name)
        elif count == 0:
            logging.error('Waiting for target VNC window, %s, to exist', quoted_window_name)
        else:
            # Command flags here: http://www.tightvnc.com/doc/win/TightVNC_2.7_for_Windows_Server_Command-Line_Options.pdf
            return r'"{TIGHTVNC_DIR}/tvnserver.exe" -controlapp -sharewindow "{windowname}"'.format(
                TIGHTVNC_DIR=TIGHTVNC_DIR,
                windowname=window_name
            )
        time.sleep(1)


def get_window_count(window_name):
    titles = enum_window_titles()
    count = titles.count(window_name)
    quoted_window_name = '"' + window_name + '"'
    return count, quoted_window_name


def configure_and_start_tightvnc(logger, windowname, port):
    # First set registry values
    set_reg_values(r'Software\TightVNC\Server', [
                          ('RfbPort', int(port))
                      ])

    # Then start the server
    logger.info(
        "Starting TightVNC Server with the following command: /n{}".format(
            start_tvnserver_command()))
    tightvnc_server = subprocess.Popen(start_tvnserver_command())

    # TODO: Check if the vncserver is active and then configure it.
    # TODO: Put this in a thread so we don't block the server
    time.sleep(1)

    # Then point it at the game
    logger.info("Configuring TightVNC Server with the following command: /n{}".format(
            configure_tvnserver_command(windowname)))
    tightvnc_config = subprocess.Popen(configure_tvnserver_command(windowname))
    return tightvnc_server, tightvnc_config


