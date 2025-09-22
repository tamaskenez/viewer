#!/bin/bash -e
cp bw/src/viewer/viewer* docs
git add docs/*
git commit -m "gh-pages updated"
git push origin head 
