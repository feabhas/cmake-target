#!/usr/bin/python3
"""
Usage: copy_solution.py [--help] [number]
Copy a specified solution replacing the current `src` and
`include` files. If `number` is not specified you can pick
from a list of available exercise solutions.

Current files are saved and committed to the git repo, or
copied to the `src.bak` folder if git is not being used.
"""
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path


WIN32 = sys.platform == 'win32'
GIT = r'"C:\Program Files\Git\bin\git.exe"' if WIN32 else 'git'


class CopyError(Exception):
    pass


class Config:
    locations = '. .. ~'.split()
    location_names = 'current folder, parent folder or home folder'
    location_patterns = '. exercises *_exercises *_exercises/exercises'.split()
    sources = 'src include'.split()
    backup = Path('src.bak')
    solutions = re.compile('^[0-9]?[0-9][A-Z]?[-_]')


def run(cmd: str, show=True, capture=False):
    if show:
        print(cmd)
    cp = subprocess.run(cmd, shell=True, check=True, text=True, stdout=(subprocess.PIPE if capture else None))
    return cp.stdout if capture else None


def try_build():
    try:
        build = False
        if Path('/.dockerenv').exists():
            build = True
        elif os.access('/bin/bash', os.X_OK):
            compiler = 'arm-none-eabi-gcc' if Path('system').is_dir() else 'gcc'
            build = bool(run(f'which {compiler}', show=False, capture=True))
        if build:
            run('./build.sh reset')
        else:
            print('\n*** You should now rebuild your application ***')
    except subprocess.SubprocessError as ex:
        print(f'{ex}\nCannot rebuild the application automatically\n\n*** You should now rebuild your application ***')


def find_solutions():
    for location in Config.locations:
        root = Path(location).expanduser()
        paths = []
        for pattern in Config.location_patterns:
            for solutions in root.glob(pattern + '/solutions'):
                if solutions.is_dir():
                    paths.append(solutions)
        if len(paths) > 1:
            raise CopyError(f'Ambiguous solution locations:\n  ' + '\n  '.join(str(p) for p in paths))
        if paths:
            path = paths[0]
            if path.is_dir():
                return path
    raise CopyError(f'Cannot find "exercises" or "solutions" folder in standard locations:\n'
                    f'  {Config.location_names}\n'
                    f'Please run the "configure.py" script to download your solutions')


def select_solution(solutions: Path, selected: str = ''):
    folders = [s for s in sorted(solutions.iterdir()) if s.is_dir() and Config.solutions.match(s.name)]
    if selected:
        choice = [f for f in folders if f.name.startswith(selected) or f.name.startswith('0'+selected)]
        if len(choice) == 1:
            return choice[0]
        print(f'Cannot find solution matching command line parameter: "{selected}"')
    print('Available solutions:')
    for option in folders:
        print(f'{option.name}')
    choice = input('Enter start of solution name (you can omit a leading zero)\nor q to quit? ').strip().lower()
    if not choice or choice.startswith('q'):
        raise CopyError('No choice made')
    found = [f for f in folders if re.match(f'^0?{choice}[-_]', f.name, re.IGNORECASE)]
    if not found:
        raise CopyError(f'Choice {choice} did not match a solution')
    elif len(found) != 1:
        raise CopyError(f'Choice {choice} matched more than one solution')
    return found[0]


def git_commit(solution: Path):
    try:
        git = Path('.git')
        if not git.exists():
            if Path('/.dockerenv').exists() or os.getenv('USER') == 'feabhas':
                print(f'Initialising git repo')
                wd = Path().absolute()
                run(f'{GIT} init -b main')
                run(f'{GIT} config --global --add safe.directory {wd}')
                run(f'{GIT} config core.safecrlf false')
                run(f'{GIT} add -A')
            else:
                return False
        print(f'Saving source files to git')
        run(f'{GIT} add -A')
        run(f'{GIT} commit -qm "Loading solution: {solution.name}"')
        return True
    except subprocess.SubprocessError as ex:
        print(f'{ex}\ngit command failed')
    return False


def save_sources(solution: Path):
    if git_commit(solution):
        return
    backup = Config.backup
    backup.mkdir(parents=True, exist_ok=True)
    print(f'Moving current source files to "{backup.name}"')
    for folder in Config.sources:
        source = Path(folder)
        target = backup / folder
        if target.exists():
            shutil.rmtree(target)
        if source.exists():
            shutil.move(source, backup)


def copy_solution(solution):
    sources = Config.sources if (solution / 'src').is_dir() else ['.']
    for folder in sources:
        source = solution / folder
        if not source.exists():
            continue
        print(f'Copying solution sources from "{source.parent.name}/{source.name}"')
        target = Path(folder if folder != '.' else 'src')
        if target.exists():
            shutil.rmtree(target)
        shutil.copytree(source, target)


def do_copy_solution(solution: str):
    solutions = find_solutions()
    solution = select_solution(solutions, solution)
    print(f'Copying solution "{solution.name}"')
    save_sources(solution)
    copy_solution(solution)
    try_build()


def cd_workspace():
    cwd = Path().absolute()
    for wd in cwd, cwd.parent:
        if (wd / 'src').exists():
            os.chdir(wd)
            return
    raise CopyError('Please run this script from within the workspace root folder')


def parse_args():
    number = ''
    for arg in sys.argv[1:]:
        if arg == '--help':
            print(__doc__, file=sys.stderr)
            exit(1)
        elif arg.startswith('--') or number:
            if number:
                print(f'Solution number "{number}" already specified', file=sys.stderr)
            else:
                print(f'Unknown argument: "{arg}"', file=sys.stderr)
            print(__doc__, file=sys.stderr)
            exit(1)
        else:
            number = arg
    return number


def main():
    status = 1
    try:
        number = parse_args()
        cd_workspace()
        do_copy_solution(number)
        status = 0
    except CopyError as ex:
        print(ex, file=sys.stderr)
    except (KeyboardInterrupt, EOFError):
        pass
    except Exception as ex:
        print(ex, file=sys.stderr)
        import traceback
        print(traceback.format_exc(), file=sys.stderr)
    if WIN32 and not os.getenv('PROMPT'):
        input('Press <Enter> to close the window')
    exit(status)


if __name__ == '__main__':
    main()
