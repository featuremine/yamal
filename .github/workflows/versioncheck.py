import os
import sys
import subprocess
from github import Github

# todo: ADD create pre-rel
# todo: ADD PRE-REL check

isnewversion = False
isbugfix = False
isversbump = False

ghclient = Github(os.getenv('GH_TOKEN'))

# Get list of already existing tags, minus the leading 'v'
releases = []
for release in ghclient.get_repo("featuremine/yamal").get_releases():
    rel = {}
    tag = str(release.tag_name[1:]).split('.')
    rel['major'] = tag[0]
    rel['minor'] = tag[1]
    rel['patch'] = tag[2]
    rel['string'] = f'{rel["major"]}.{rel["minor"]}.{rel["patch"]}'
    if len(tag) == 4:
        rel['bugfix'] = tag[3]
        rel['string'] = f'{rel["string"]}.{rel["bugfix"]}'
    releases.append(rel)

# Iterate through list for latest (highest) tag
latest = releases[0]
for release in releases:
    if int(release['major']) > int(latest['major']):
        latest = release
        continue
    elif int(release['major']) == int(latest['major']):
        if int(release['minor']) > int(latest['minor']):
            latest = release
            continue
        elif int(release['minor']) == int(latest['minor']):
            if int(release['patch']) > int(latest['patch']):
                latest = release
                continue
print(f'Latest release is {latest["string"]}.')

with open('../../VERSION', encoding = 'utf-8') as versionFile:
    current = str(versionFile.read()).splitlines()[0].split('.')
    cur = {}
    cur['major'] = current[0]
    cur['minor'] = current[1]
    cur['patch'] = current[2]
    cur['string'] = f'{cur["major"]}.{cur["minor"]}.{cur["patch"]}'
    if len(current) == 4:
        cur['bugfix'] = current[3]
        cur['string'] = cur["string"] + f'.{cur["bugfix"]}'
    print(f'Version in file is {cur["string"]}.')

# Is cur in releases?
for release in releases:
    if release['major'] == cur['major'] and release['minor'] == cur['minor'] and release['patch'] == cur['patch'] :
        isnewversion = False
        if len(release) == 4 and len(cur) == 5:
            isbugfix = True
            isversbump = True
        elif len(release) == 5 and len(cur) == 5:
            if int(cur["bugfix"]) > int(release["bugfix"]):
                isbugfix = True
                isversbump = True
        break
    else:
        isnewversion = True

# Was the version bumped?
if isnewversion:
    # Compare current vs latest
    if int(cur['major']) > int(latest['major']):
        isversbump = True
        print("Major version change")
    elif int(cur['major']) == int(latest['major']):
        if int(cur['minor']) > int(latest['minor']):
            isversbump = True
            print("Minor version change")
        elif int(cur['minor']) == int(latest['minor']):
            if int(cur['patch']) > int(latest['patch']):
                isversbump = True
                print("Patch version change")


base = os.getenv('BASE')
print(f'Destination branch is {base}')
if base == 'main':
    if isnewversion and isversbump:
        print(f'Releasing new version {cur["string"]}.')
        sys.exit(0)
    elif isbugfix:
        print(f'Error, cannot merge bugfix {cur["string"]} into main.')
        sys.exit(4)
    else:
        print(f'Fix version in version file before merging to main.')
        sys.exit(1)
    
else:
    if isbugfix and isversbump:
        print(f'Releasing bug fix {cur["string"]}.')
        sys.exit(3)
    else:
        print(f'Non-release event is not processed by this workflow.')
        sys.exit(2)
