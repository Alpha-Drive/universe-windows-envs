import os
import sys
import logging
import shutil
import six

CURR_DIR = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(os.path.abspath(os.path.join(CURR_DIR, os.pardir)), 'vnc-windows'))
import gym_windows

logger = logging.getLogger()

UNIVERSE_BACKUP_NAME = 'universe-windows-envs'
UNIVERSE_WINDOWS_ENVS_DIR = os.environ['UNIVERSE_WINDOWS_ENVS_DIR']
BACKUP_DIR = os.path.expanduser('~\\Documents\\Universe\\envs-update-backup')


def backup_update_executables():
    gym_windows.run_win_cmd('rmdir %s /s /q' % BACKUP_DIR)
    backed_up = False
    for dirpath, dnames, fnames in os.walk(UNIVERSE_WINDOWS_ENVS_DIR):
        for fname in fnames:
            if fname.endswith('.py'):
                old_path = os.path.join(dirpath, fname)
                rel_path = old_path[len(UNIVERSE_WINDOWS_ENVS_DIR):]
                new_path = BACKUP_DIR + rel_path
                new_dir = os.path.dirname(new_path)
                if not os.path.exists(new_dir):
                    os.makedirs(new_dir)
                shutil.copy2(old_path, new_path)
                backed_up = True

    if not backed_up:
        raise Exception('No files found in %s pointed to by the %s environment variable' % (
            UNIVERSE_WINDOWS_ENVS_DIR, 'UNIVERSE_WINDOWS_ENVS_DIR'))


def try_updating(update_fn, restore_fn):
    try:
        update_fn()
    except:
        exc_info = sys.exc_info()
        try:
            restore_fn()
        except:
            # If this happens, it clobbers exc_info, which is why we had
            # to save it above
            import traceback
            logger.error('Error updating. Please reinstall universe-windows-envs')
            traceback.print_exc()
            six.reraise(*exc_info)
        logger.error('Error updating, please retry. (You may need to close programs with universe-windows-envs files'
                     ' open). See below for more details.')
        six.reraise(*exc_info)


def update():
    logger.info('Updating windows environments, download SIZE is ~2GB')
    gym_windows.download_folder('https://www.dropbox.com/s/ljx7uiodptxr0f3/universe-windows-envs.zip?dl=1',
                                os.path.dirname(os.environ['UNIVERSE_WINDOWS_ENVS_DIR']), warn_existing=False)


def restore():
    shutil.copytree(BACKUP_DIR, UNIVERSE_WINDOWS_ENVS_DIR)


def main():
    backup_update_executables()  # In case the update fails, we can restore these and try again
    gym_windows.run_win_cmd('rmdir %s /s /q' % UNIVERSE_WINDOWS_ENVS_DIR)
    try_updating(update, restore_fn=restore)
    logger.info('Update complete')


if __name__ == '__main__':
    logging.basicConfig()
    logger.setLevel(logging.INFO)
    sys.exit(main())
