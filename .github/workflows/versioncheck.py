import os
import sys
import subprocess
from github import Github

isnewversion = False
isbugfix = False
isversbump = False

ghclient = Github(os.getenv('TOKEN'))

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
        rel['string'] = rel["string"] + f'.{rel["bugfix"]}'
    releases.append(rel)

# Iterate through list for latest (highest) tag
latest = releases[0]
for release in releases:
    if int(release['major']) > int(latest['major']):
        latest = release
        continue
    else:
        if int(release['minor']) > int(latest['minor']):
            latest = release
            continue
        else:
            if int(release['patch']) > int(latest['patch']):
                latest = release
                continue
print(f'Latest release is {latest["string"]}.')

versionFile = open('../../VERSION')
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
    if release['string'] in cur['string']:
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

# Is cur in releases?
if isnewversion:
    # Compare current vs latest
    if int(cur['major']) > int(latest['major']):
        isversbump = True
        print("Major version change")
    else:
        if int(cur['minor']) > int(latest['minor']):
            isversbump = True
            print("Minor version change")
        else:
            if int(cur['patch']) > int(latest['patch']):
                isversbump = True
                print("Patch version change")


base = os.getenv('BASE')
print(f'Destination branch is {base}')
if base == 'main':
    if isnewversion and isversbump:
        print(f'Releasing new version {cur["string"]}.')
        sys.exit(0)
    else:
        print(f'Correct version in version file before merging to main.')
        sys.exit(1)
    
else:
    if isbugfix and isversbump:
        print(f'Releasing bug fix {cur["string"]}.')
        sys.exit(3)
        
    else:
        print(f'Non-release event is not processed by this workflow.')
        sys.exit(2)
