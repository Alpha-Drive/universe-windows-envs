#!/usr/bin/python
from __future__ import print_function
import argparse
import json
import logging
import os
import subprocess
import sys
import csv
import traceback
import urllib
import urllib2
import zipfile
from sys import platform as _platform
import time
import six

# TODO: Rearrange directories for a proper import
CURRENT_DIR = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(os.path.abspath(os.path.join(CURRENT_DIR, os.pardir)), 'vnc-windows'))
import gym_windows
from enforce_version import enforce_version

logger = logging.getLogger(__name__)

### Constants ####

GTAV_DIR = os.environ['GTAV_DIR']
GTAV_CONTROLLER_DIR = CURRENT_DIR + r"/GTAVController"
INJECT_DIR = GTAV_CONTROLLER_DIR + r"/inject"
IS_STEAM = 'steam' in GTAV_DIR.lower()
CONTROLLER_PROCESS_NAME = 'GTAVController.exe'
TIGHTVNC_PROCESS_NAME = 'tvnserver.exe'
GTAV_PROCESS_NAME = 'GTA5.exe'
ALL_PROCESS_NAMES = [TIGHTVNC_PROCESS_NAME, CONTROLLER_PROCESS_NAME, GTAV_PROCESS_NAME]


def _start_gtav_command():
    """
    This command needs to change directory to GTAV_DIR in order to properly find the maps that we need.
    Therefore it always needs to be called with shell=True

    Injectory command flags here: https://github.com/blole/injectory/tree/651700018750c4f2003f1f048b090c4d521717a6
    """
    if IS_STEAM:
        start_command = r'start steam://rungameid/271590'
    else:
        start_command = r'"{GTAV_DIR}/PlayGTAV.exe"'.format(GTAV_DIR=GTAV_DIR)

    return start_command


def _gtav_is_running():
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


# TODO: Create base class in gym_windows
class GTAVEnvRunner(object):
    def __init__(self, env_id, instance_id, skip_loading_saved_game, pause_for_attach, rewards_per_second, scenario):
        if _platform != "win32":
            raise AttributeError("GTAV only runs on Windows")
        self.env_id = env_id
        self.instance_id = instance_id
        self.skip_loading_saved_game = skip_loading_saved_game
        self.pause_for_attach = pause_for_attach
        self.vnc_port = 5900
        self.websocket_port = 15900
        self.rewards_per_second = rewards_per_second
        self.scenario = scenario
        self.tightvnc_server = None
        self.tightvnc_config = None
        self.gtav_proc = None
        self.gtav_controller = None

    def _configure(self):
        self.popen()

    def popen(self):
        subprocess.call("taskkill /IM %s /F" % CONTROLLER_PROCESS_NAME)

        if _gtav_is_running():
            logger.info("GTAV already running. Will use existing instance: /n{}".format(_start_gtav_command()))
        else:
            self._kill_competing_procs()
            # Kill existing servers that may be running
            logger.info("Starting GTAV with the following command: /n{}".format(_start_gtav_command()))
            self.gtav_proc = subprocess.Popen(_start_gtav_command(), shell=True)  # Needs shell for `cd`
            while not gym_windows.processes_are_running(GTAV_PROCESS_NAME):
                logger.info('Waiting for %s to start', GTAV_PROCESS_NAME)

        subprocess.call("taskkill /IM %s /F" % TIGHTVNC_PROCESS_NAME)
        self.tightvnc_server, self.tightvnc_config = gym_windows.configure_and_start_tightvnc(
            logger, 'Grand Theft Auto V', self.vnc_port)

        gtav_controller_command = self._start_gtav_controller_command()
        gtav_env = self.get_gtav_env()
        logger.info("Starting GTAVController with the following command: {command}".
                    format(command=gtav_controller_command))
        self.gtav_controller = subprocess.Popen(gtav_controller_command, shell=True, env=gtav_env)

    def _start_gtav_controller_command(self):
        return r'"{GTAV_CONTROLLER_DIR}/bin/{CONTROLLER_PROCESS_NAME}" {env_id} {instance_id} {websocket_port} {rewards_per_second} {skip_loading_saved_game} {pause_for_attach} {scenario}'.format(
            GTAV_CONTROLLER_DIR=GTAV_CONTROLLER_DIR,
            env_id=self.env_id,
            instance_id=self.instance_id,
            websocket_port=self.websocket_port,
            skip_loading_saved_game=json.dumps(self.skip_loading_saved_game),
            rewards_per_second=self.rewards_per_second,
            pause_for_attach=json.dumps(self.pause_for_attach),
            scenario=self.scenario,
            CONTROLLER_PROCESS_NAME=CONTROLLER_PROCESS_NAME,
        )

    @staticmethod
    def _kill_competing_procs():
        subprocess.call("taskkill /IM %s /F" % TIGHTVNC_PROCESS_NAME)
        subprocess.call("taskkill /IM %s /F" % CONTROLLER_PROCESS_NAME)
        # Don't kill steam as this can require restart to get GTA running again

    def popen_cleanup(self):
        for proc in [self.tightvnc_server, self.tightvnc_config, self.gtav_controller]:
            if proc:
                proc.kill()
                proc.wait()

        # TODONT kill self.gta_proc as we can reuse the process and it takes ~1 minute to start

        # Kill procs that we might have missed
        # TODO: Remove this for multitenancy
        self._kill_competing_procs()

    def get_gtav_env(self):
        env = os.environ.copy()
        env['PATH'] += r';C:\Program Files\vJoy\x86;C:\Program Files (x86)\AutoIt3\AutoItX;'
        return env


def try_stuff(fn, clean):
    try:
        fn()
    except:
        exc_info = sys.exc_info()
        try:
            clean()
        except:
            # If this happens, it clobbers exc_info, which is why we had
            # to save it above
            import traceback
            logger.error('Error cleaning up gtav env processes')
            traceback.print_exc()
        six.reraise(*exc_info)


def send_hearbeat(last_ping):
    ping_diff = None
    ping_period_seconds = 15 * 60
    if last_ping is not None:
        ping_diff = time.time() - last_ping
    if ping_diff is None or ping_diff > ping_period_seconds:
        logging.info('Sending heartbeat')
        try:
            last_ping = time.time()
            urllib2.urlopen(os.environ['GTAV_DEAD_MANS_SNITCH_URL']).read()
        except Exception as e:
            logging.error('Error sending heartbeat \n' + traceback.format_exc())
    return last_ping


def main():
    parser = argparse.ArgumentParser(description=None)
    parser.add_argument('-v', '--verbose', action='count', dest='verbosity', default=0, help='Set verbosity.')
    parser.add_argument('-e', '--env_id', default='gtav.SaneDriving-v0',
                        help='What GTAV sub-environment do you want to run?')
    parser.add_argument('-s', '--skip-loading-saved-game', action='store_true',
                        help='Do you want to skip reloading the saved game (useful for debugging)?')
    parser.add_argument('-p', '--pause-for-attach', action='store_true',
                        help='Do you want to pause the controller before running the environment so we can attach a debugger?')
    parser.add_argument('-i', '--instance_id', default='env_PLACEHOLDER_ID',
                        help='What is the id of this instance of the environment?')
    parser.add_argument('-r', '--rewards_per_second', default=8., help='How many rewards per second?')
    parser.add_argument('-sc', '--scenario', default='', help='The scenario you want to run')

    args = parser.parse_args()

    logging.basicConfig()

    if args.verbosity == 0:
        logger.setLevel(logging.INFO)
    elif args.verbosity >= 1:
        logger.setLevel(logging.DEBUG)

    enforce_version(GTAV_DIR)

    runner = GTAVEnvRunner(args.env_id, args.instance_id, args.skip_loading_saved_game, args.pause_for_attach,
                           args.rewards_per_second, args.scenario)

    try_stuff(runner.popen, runner.popen_cleanup)

    last_heartbeat = None
    while True:
        running_procs = gym_windows.get_running_processes(ALL_PROCESS_NAMES)
        if len(running_procs) == len(ALL_PROCESS_NAMES):
            print('processes running weeeee')
            if 'GTAV_DEAD_MANS_SNITCH_URL' in os.environ:
                last_heartbeat = send_hearbeat(last_heartbeat)
        else:
            missing = str(list(set(ALL_PROCESS_NAMES) - set(running_procs)))
            runner.popen_cleanup()
            raise Exception('Environment crashed due to the following failed processes: ' + missing)
        time.sleep(5)


if __name__ == '__main__':
    sys.exit(main())
