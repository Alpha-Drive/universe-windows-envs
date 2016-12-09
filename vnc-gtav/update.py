import os
import sys
CURR_DIR = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(os.path.abspath(os.path.join(CURR_DIR, os.pardir)), 'vnc-windows'))
import gym_windows
gym_windows.download_folder('https://www.dropbox.com/s/ljx7uiodptxr0f3/universe-windows-envs.zip?dl=1',
                            os.environ('UNIVERSE_WINDOWS_ENVS_DIR'))

