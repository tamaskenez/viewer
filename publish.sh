#!/bin/bash -e
cp bw/src/viewer/viewer* gh-pages
git add gh-pages/*
git commit -m "gh-pages updated"
git push origin head 
