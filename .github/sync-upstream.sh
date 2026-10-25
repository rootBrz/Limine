#!/bin/sh
set -e

upstream_url=${UPSTREAM_URL:-https://github.com/Limine-Bootloader/Limine.git}
upstream_branch=${UPSTREAM_BRANCH:-v12.x}
branch=${BRANCH:-uefi-gamepad}

git remote add upstream "$upstream_url"
git fetch --quiet upstream "$upstream_branch"

if git merge-base --is-ancestor FETCH_HEAD HEAD; then
    echo "Already up to date."
    exit 0
fi

git merge --no-edit -m "Merge upstream $upstream_branch" FETCH_HEAD

# The workflow token may not modify workflow files, so keep this branch's.
git rm -rfq --ignore-unmatch .github/workflows
git checkout HEAD^1 -- .github/workflows
git diff --cached --quiet || git commit -q --amend --no-edit

git push origin "HEAD:refs/heads/$branch"
