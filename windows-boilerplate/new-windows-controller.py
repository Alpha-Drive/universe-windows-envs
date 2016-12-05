#!/usr/bin/python
from __future__ import unicode_literals
import argparse
import codecs
import fnmatch
import logging
import os
import re
import shutil
import sys
import uuid

import jinja2

logger = logging.getLogger(__name__)

TEMPLATE_PATH = os.path.join(os.path.abspath(os.path.dirname(os.path.realpath(__file__))), 'vnc-windows-template')
VISUAL_STUDIO_GUID = '8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942'
COMMON_CONTROLLER_SOLUTION_GUID = '15902083-70AA-44BE-AF74-A30631860DC8'
SOLUTION_PROJECT = r"""Project("{{ visual_studio_guid }}") = "{{ proj_camel_case }}Controller", "..\..\..\vnc-{{ proj_dash_case }}\{{ proj_camel_case }}Controller\VisualStudio\{{ proj_camel_case }}Controller.vcxproj", "{{ proj_guid }}"
	ProjectSection(ProjectDependencies) = postProject
		{{ common_controller_solution_guid }} = {{ common_controller_solution_guid }}
	EndProjectSection
EndProject"""
SOLUTION_PROJECT_CFG = r"""		{{ proj_guid }}.Debug|Win32.ActiveCfg = Debug|Win32
		{{ proj_guid }}.Debug|Win32.Build.0 = Debug|Win32
		{{ proj_guid }}.Release|Win32.ActiveCfg = Release|Win32
		{{ proj_guid }}.Release|Win32.Build.0 = Release|Win32"""


def camel_to_snake_case(name):
    s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
    return re.sub('([a-z0-9])([A-Z])', r'\1_\2', s1).lower()


def find_files_and_dirs(directory, pattern, return_dirs):
    def find_stuff(things):
        for basename in things:
            if fnmatch.fnmatch(basename, pattern):
                filename = os.path.abspath(os.path.join(root, basename))
                yield filename
    for root, dirs, files in os.walk(directory):
        if return_dirs:
            for d in find_stuff(dirs): yield d
        else:
            for f in find_stuff(files): yield f


def find_files(directory, pattern):
    return find_files_and_dirs(directory, pattern, return_dirs=False)


def find_dirs(directory, pattern):
    return find_files_and_dirs(directory, pattern, return_dirs=True)


def replace_dir_names(directory, old, new):
    for d in find_dirs(directory, '*%s*' % old):
        os.rename(d, d.replace(old, new))


def replace_file_names(directory, old, new):
    for f in find_files(directory, '*%s*' % old):
        os.rename(f, f.replace(old, new))


def replace_templates(dest, proj_camel_case, proj_lower_case, proj_shout_case,
                      proj_dash_case, proj_guid, resource_files_guid,
                      source_files_guid, vendor_files_guid, header_files_guid):
    for f in find_files(dest, '*'):
        with codecs.open(f, "r", 'utf-8') as infile:
            contents = infile.read()
        new_content = jinja2.Template(contents).render({
            'proj_camel_case': proj_camel_case,
            'proj_guid': '{%s}' % proj_guid,
            'proj_lower_case': proj_lower_case,
            'proj_shout_case': proj_shout_case,
            'proj_dash_case': proj_dash_case,
            'resource_files_guid': '{%s}' % resource_files_guid,
            'source_files_guid': '{%s}' % source_files_guid,
            'vendor_files_guid': '{%s}' % vendor_files_guid,
            'header_files_guid': '{%s}' % header_files_guid
        })
        with codecs.open(f, 'w', 'utf-8') as outfile:
            outfile.write(new_content)


def add_before_or_after_last(add, search, filename, after):
    with codecs.open(filename, "r", 'utf-8') as f1:
        contents = f1.read()
    index = contents.rfind(search)
    contents = list(contents)
    if after:
        index += len(search)
    contents.insert(index, add)
    with codecs.open(filename, "w", 'utf-8') as f2:
        contents = ''.join(contents)
        f2.write(contents)


def add_after_last(add, search, filename):
    add_before_or_after_last(add, search, filename, True)


def add_before_last(add, search, filename):
    add_before_or_after_last(add, search, filename, False)


def main():
    parser = argparse.ArgumentParser(description=None)
    parser.add_argument('-v', '--verbose', action='count', dest='verbosity', default=0, help='Set verbosity.')
    parser.add_argument('-e', '--name', default='AwesomeGame', help='What is the name of the game in camel case?')
    args = parser.parse_args()

    proj_camel_case = args.name.replace(' ', '')
    proj_shout_case = camel_to_snake_case(proj_camel_case).upper()
    proj_lower_case = camel_to_snake_case(proj_camel_case).lower()
    proj_dash_case = proj_lower_case.replace('_', '-')
    proj_guid = str(uuid.uuid4()).upper()
    header_files_guid = str(uuid.uuid4()).upper()
    resource_files_guid = str(uuid.uuid4()).upper()
    source_files_guid = str(uuid.uuid4()).upper()
    vendor_files_guid = str(uuid.uuid4()).upper()

    if args.verbosity == 0:
        logger.setLevel(logging.INFO)
    elif args.verbosity >= 1:
        logger.setLevel(logging.DEBUG)

    logging.basicConfig()

    dest = os.path.abspath('{TEMPLATE_PATH}/../../vnc-{proj_dash_case}'.format(
        TEMPLATE_PATH=TEMPLATE_PATH, proj_dash_case=proj_dash_case))

    if os.environ.get('DELETE', False) and os.path.exists(dest):
        # For quick testing
        logging.warn('Deleting existing contents of ' + dest)
        shutil.rmtree(dest)

    shutil.copytree(TEMPLATE_PATH, dest)
    replace_dir_names(dest, 'WindowsTemplate', proj_camel_case)
    replace_file_names(dest, 'WindowsTemplate', proj_camel_case)
    replace_templates(dest, proj_camel_case, proj_lower_case, proj_shout_case,
                      proj_dash_case, proj_guid, resource_files_guid,
                      source_files_guid, vendor_files_guid, header_files_guid)

    solution_file = list(find_files('{TEMPLATE_PATH}/../../'.format(
        TEMPLATE_PATH=TEMPLATE_PATH), 'CommonController.sln'))[0]

    sln_proj_add = jinja2.Template(SOLUTION_PROJECT).render({
        'visual_studio_guid': '{%s}' % VISUAL_STUDIO_GUID,
        'proj_guid': '{%s}' % proj_guid,
        'common_controller_solution_guid': '{%s}' % COMMON_CONTROLLER_SOLUTION_GUID,
        'proj_camel_case': proj_camel_case,
        'proj_dash_case': proj_dash_case
    }) + '\r\n'
    add_after_last(sln_proj_add, 'EndProject\r\n', solution_file)
    sln_proj_cfg_add = jinja2.Template(SOLUTION_PROJECT_CFG).render({
        'proj_guid': '{%s}' % proj_guid
    }) + '\r\n'
    add_after_last(sln_proj_cfg_add, 'GlobalSection(ProjectConfigurationPlatforms) = postSolution\r\n', solution_file)
    logger.info("Created new project: %s in %s" % (proj_camel_case, dest))
    return 0

if __name__ == '__main__':
    sys.exit(main())
