#!/bin/bash

make

REPO=4ca309a1857158d5766d4ede4235cae0
URL=https://gist.github.com/bobbae

#download from the net if the file does not exist 
if [ ! -f "words.txt" ]
then
	echo "No words.txt --- downloading from git."
	wget $URL/$REPO/archive/master.zip
	unzip master.zip
   	mv $REPO-master/words.txt . 
	rm master.zip 
	rm -rf $REPO-master
fi 
#download file from internet 
#if no input argument, file "words.txt" will be used by default.  
echo -e "\n\n\nStart find longest compound word!!..."
./find_compound $1
