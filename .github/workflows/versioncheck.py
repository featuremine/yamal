#!/usr/bin/env python3
import os
import sys
import subprocess
from os import path
from github import Github

def version_file():
    where = path.dirname(path.dirname(path.dirname(__file__)))
    return path.join(where, 'VERSION')

class Version(object):
    def __init__(self, version_str):
        self._numbers = [int(x) for x in str(version_str).split('.')]

    def regular(self):
        return len(self._numbers) == 3

    def bugfix(self):
        return len(self._numbers) == 4

    def __lt__(self, other):
        for x, y in zip(self._numbers, other._numbers):
            if x < y:
                return True
        
        return len(self._numbers) < len(other._numbers)
    def __str__(self):
        return '.'.join([str(x) for x in self._numbers])

def version():
    return Version(list(open(version_file()))[0].strip())

def version_check():
    res = subprocess.run(['git', 'diff', os.getenv('GIT_MERGE_BASE'), '--', version_file()], capture_output=True)
    assert not res.returncode, f'git diff failed to execute with output\n{res.stderr}'
    if not res.stdout:
        return None
    return version()

ghrepo = Github(os.getenv('GH_TOKEN')).get_repo("featuremine/yamal")

def releases():
    rels = []
    for release in ghrepo.get_releases():
        if not release.draft:
            rels.append(Version(release.tag_name[1:]))

    return sorted(rels, reverse=True)        

def main_merge():
    return os.getenv('GIT_MERGE_BASE') == 'main'


if __name__ == '__main__':
    ver = version_check()
    rels = releases()
    version_bump = ver and ver.regular() and (not rels or rels[0] < ver)
    regular_pr = not ver and not main_merge()
    proper_release = main_merge() and version_bump
    # TODO need to check that bug fix is proper
    # derived from existing release and large than existing bug fixes
    bug_fix_release = ver and ver.bugfix() and not main_merge()
    good_to_go = regular_pr or proper_release or bug_fix_release
    if not good_to_go:
        if main_merge():
            raise "Merging to main allowed with a proper version bump only"
        elif ver and not ver.bug_fix():
            raise "Version changes other than bug fixes need to increase the version and merge into main"
        else:
            raise "Bug fix needs to extend an existing release and must increase bug fix version"

    if proper_release or bug_fix_release:
        #create draft
        # check if draft release exists
        #if it does, amment it by changing target_commitish
        #else create new
        ghrepo.create_git_release(f'v{ver}', f'v{ver}', f'draft release for version v{ver}',
            draft=True, prerelease=False, target_commitish=os.getenv('GIT_MERGE_COMMIT'))
        print("::set-output name=release::release")
    else:
        print("::set-output name=release::regular")
