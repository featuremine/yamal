python3 versioncheck.py;

if [ $? -eq 0 ]; then 
    echo "Releasing new version v$(cat ../../VERSION)"
    gh release create v$(cat ../../VERSION) --prerelease
elif [ $? -eq 1 ]; then 
    echo "Fix version in version file before merging to main."
elif [ $? -eq 2 ]; then 
    echo "Non-release event is not processed by this workflow."
elif [ $? -eq 3 ]; then 
    echo "Releasing bug fix v$(cat ../../VERSION)"
    gh release create v$(cat ../../VERSION) --prerelease
elif [ $? -eq 4 ]; then 
    echo "Error, cannot merge bugfix v$(cat ../../VERSION) into main."
fi 
