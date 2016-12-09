import os
import sys
import logging
CURR_DIR = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(os.path.abspath(os.path.join(CURR_DIR, os.pardir)), 'vnc-windows'))
import gym_windows

logger = logging.getLogger()

def main():
    gym_windows.download_folder('https://www.dropbox.com/s/ljx7uiodptxr0f3/universe-windows-envs.zip?dl=1',
                                os.path.dirname(os.environ['UNIVERSE_WINDOWS_ENVS_DIR']))
    logger.info('Update complete')


if __name__ == '__main__':
    logging.basicConfig()
    logger.setLevel(logging.INFO)
    sys.exit(main())
