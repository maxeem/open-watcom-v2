#!/bin/sh
# *****************************************************************
# clrgittr.sh - get clone of GitHub repository to return Travis CI
#               OW build/log files and compress GitHub repository
#               if necessary to hold only a last 10 builds
# *****************************************************************
#
# configure Git client
#

case "$OWTRAVISJOB" in
  BOOTLINUX|BUILDLINUX|BOOTOSX|PDFDOCLINUX)
    if [ "$TRAVIS_PULL_REQUEST" = "false" ]; then
        git config --global user.email "travis@travis-ci.org"
        git config --global user.name "Travis CI"
        git config --global push.default simple
        #
        # clone GitHub repository
        #
        git clone --quiet --branch=master https://${GITHUB_TOKEN}@github.com/open-watcom/travis-ci-ow-builds.git ${OWRELROOT}
        #
        # compress GitHub repository to hold only a few last builds
        #
        cd $OWRELROOT
        depth=`git rev-list HEAD --count`
        if [ $depth -gt 10 ]; then
            echo "gitget.sh - start compression"
            git checkout --orphan temp1
            git add -A
            git commit -am "Initial commit"
            git branch -D master
            git branch -m master
            git push -f origin master
            git branch --set-upstream-to=origin/master master
            git pull
            echo "gitget.sh - end compression"
        fi
        cd $TRAVIS_BUILD_DIR
        echo "gitget.sh - done"
    else
        echo "gitget.sh - skipped"
    fi
    ;;
  *)
    echo "gitget.sh - skipped"
    ;;
esac
