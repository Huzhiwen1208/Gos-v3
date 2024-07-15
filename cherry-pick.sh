#!/bin/bash

# This script is used to cherry-pick a commit from one branch to another.
# It takes two arguments:
# 1. The commit hash to cherry-pick
# 2. The branch to cherry-pick the commit to

# Check if the user has provided the correct number of arguments
if [ $# -ne 2 ]; then
    echo "Usage: $0 <commit-hash> <branch>"
    exit 1
fi

# Check if the commit hash is valid
if ! git cat-file -e $1^{commit} 2>/dev/null; then
    echo "Invalid commit hash: $1"
    exit 1
fi

# Check if the branch exists
if ! git show-ref --verify --quiet refs/heads/$2; then
    echo "Branch $2 does not exist"
    exit 1
fi

# Cherry-pick the commit to the specified branch
git checkout $2
git cherry-pick $1

# Check if the cherry-pick was successful
if [ $? -ne 0 ]; then
    echo "Cherry-pick failed"
    exit 1
fi

echo "Cherry-pick successful"

# push the changes to the remote branch
git push

# Check if the push was successful
if [ $? -ne 0 ]; then
    echo "Push failed"
    exit 1
fi

echo "Push successful"

git checkout master

exit 0

