#!/usr/bin/python3
"""
Usage: configure.py [--help] [--target|--host] [code]
Configure workspace for a given course by downloading the relevant 
GitHub repo and initialising the workspace with a clean git repo 
if no `.git` folder exists.

If command line `code` is not supplied the script prompts to choose 
from a list of available course codes based on the workspace 
configuration (host or embedded target).

The `--host` or `--target` saves to the current working folder
and is intended for administration of a VM image.
"""
import os
import re
import shutil
import ssl
import sys
import urllib.request
import urllib.error
from zipfile import ZipFile
from io import BytesIO
from pathlib import Path


class ConfigureError(Exception):
    pass


class Config:
    repo_suffix = '_exercises'
    url_code_base = 'http://codeload.github.com/feabhas/'
    url_code_path = '/zip/refs/heads/main'
    branch = 'main'
    course_pattern = re.compile('^[a-z][-a-z+0-9]*-[0-9]{3}$', re.IGNORECASE)
    exercise_pattern = re.compile(f'^(exercises|solutions|.*{repo_suffix})')


class Courses:
    target = [
        'C-501      "C for Real - Time Embedded Developers"',
        'AC-401     "Advanced C Programming"',
        'C++11-501  "Modern C++ for Embedded Systems (C++11/14/17)"',
        'AC++11-501 "Advanced Modern C++ for Embedded Developers (C++11/14/17)"',
    ]
    host = [
        'C++11-502        "Real-Time Modern C++ (C++11/14/17")',
        'AC++11-401       "Transitioning to Modern C++ (C++11/14/17)"',
        'AC++11-502       "Advanced Real-Time Modern C++ (C++11/14/17)"',
        'AC++11-CACHE-502 "Advanced Modern C++ Customised for Cache & Performance"',
        'AC++20-302       "Migrating to C++20/23"',
        'TDDC-301         "TDD for Embedded C"',
        'TDDC++-301       "TDD for Embedded C++"',
        'DP11-403         "Design Patterns in Modern C++"',
    ]


def read_course_code(selected: str = '') -> str:
    if selected:
        print(f'Using command line argument "{selected}"')
        course = selected
    else:
        print('If you know your course code type it now or just\n'
              'press the <Enter> key to see a list of available courses.')
        course = input('? ').strip().lower()
    if not course or Config.course_pattern.search(course):
        return course
    print(f'"{course}" does not look like a valid course code.\n'
          f'A typical course code looks like C-501, AC++11-502 or TDD++-301')


def choose_course(options: list) -> str:
    print('Suggested courses:')
    for n, option in enumerate(options, 1):
        print(f'{n:2d}) {option}')
    choice = input('Type course code, choice as a number, or q to quit? ').strip().lower()
    if not choice or choice.startswith('q'):
        return ''
    if choice.isdigit():
        n = int(choice) - 1
        if 0 <= n < len(options):
            course = options[n]
            return course.split()[0]
    return choice


def check_exercise_exists(path):
    if path.exists():
        print(f'Solutions folder "{path.name}" already exists')
        choice = input('Do you want to replace this folder [y/N]? ').strip().lower()
        if not choice.startswith('y'):
            raise ConfigureError(f'{path.name} already present')
        shutil.rmtree(path)
    exercises = [p for p in sorted(path.parent.iterdir()) if Config.exercise_pattern.search(p.name)]
    if exercises:
        print('The following exercise folders are already present:')
        print(' ', ', '.join(e.name for e in exercises))
        choice = input('Do you want to delete these folders [y/N]? ').strip().lower()
        if not choice.startswith('y'):
            raise ConfigureError(f'You can only have one set of exercises in the workspace')
        for ex in exercises:
            shutil.rmtree(ex)


def save_archive(course: str, repo: Path, url: str):
    try:
        print(f'Downloading archive "{repo.name}.zip"\n  {url}')
        with urllib.request.urlopen(url) as fp:
            zipfile = ZipFile(BytesIO(fp.read()))
            for name in zipfile.namelist():
                if name.startswith('.git'):
                    continue
                zipfile.extract(name)
        unzip = Path() / f'{repo}-{Config.branch}'
        unzip.rename(repo)
    except ssl.SSLError as ex:
        raise ConfigureError(f'''{ex}
This is a known issue for macOS users which is documented in 
    /Applications/Python 3.6/ReadMe.rtf
Just browse to "Applications/Python 3.6" and double-click "Install Certificates.command"''')
    except urllib.error.HTTPError:
        raise ConfigureError(f'Cannot find GitHub repo for course "{course}"\n'
                             f'Please check your spelling or ask your instructor for help')


def download_course(course: str) -> Path:
    repo = course + Config.repo_suffix
    url = Config.url_code_base + repo + Config.url_code_path
    path = Path(repo)
    check_exercise_exists(path)
    save_archive(course, path, url)
    return path


def course_repo(code: str) -> str:
    return code.replace('++', 'pp').lower()


def do_fetch_exercises(target: bool, selected: str) -> Path:
    course = read_course_code(selected)
    if not course:
        course = choose_course(Courses.target if target else Courses.host)
    if not course:
        raise ConfigureError('No course chosen')
    repo = course_repo(course)
    return download_course(repo)


def cd_workspace() -> bool:
    cwd = Path().absolute()
    for wd in cwd, cwd.parent:
        if (wd / 'src').exists():
            os.chdir(wd)
            return (wd / 'system').is_dir()
    raise ConfigureError('Please run this script from within the workspace root folder')


def parse_args():
    code = ''
    target = False
    dir_check = True
    for arg in sys.argv[1:]:
        if arg == '--target':
            target, dir_check = True, False
        elif arg == '--host':
            dir_check = False
        elif arg.startswith('--') or code:
            if code:
                print(f'Course code "{code}" already specified', file=sys.stderr)
            elif arg != '--help':
                print(f'Unknown argument: "{arg}"', file=sys.stderr)
            print(__doc__, file=sys.stderr)
            exit(1)
        else:
            code = arg
    return code, target, dir_check


def main():
    status = 1
    try:
        code, target, dir_check = parse_args()
        if dir_check:
            target = cd_workspace()
        repo = do_fetch_exercises(target, code)
        print('\nCourse exercises configured OK')
        status = 0
    except ConfigureError as ex:
        print('\n', ex, sep='', file=sys.stderr)
        print('Course exercises have NOT been configured')
    except (KeyboardInterrupt, EOFError):
        pass
    except Exception as ex:
        print(ex, file=sys.stderr)
        import traceback
        print(traceback.format_exc(), file=sys.stderr)
    if sys.platform == 'win32' and not os.getenv('PROMPT'):
        input('Press <Enter> to close the window')
    exit(status)


if __name__ == '__main__':
    main()
