#!/bin/bash -e
cp bw/src/viewer/viewer* docs
mv docs/viewer.html docs/index.html
git add docs/*
git commit -m "gh-pages updated"
git push origin head 
