# This script assumes that your wish the convert the
# files in your current directory. As such it takes no arguments on
# the command line.
# Script relies on SOX being installed on your computer and in your path.

# First find all audio files and converte them to 44k mono wav files
shopt -s nullglob

# echo "in x.sh with " $PWD
# echo "in 2 arg '{}' yes"
# for i in $*; do 
#   echo "arg: " $i 
# done

    FVAR = "abc"
    echo "script invoked on: " $1
   
## for file in $.wav $.ogg, $.flac; do

# for file in $1/*.flac; do
#    name=${file##*/}
#    fullname = "$1/$name"
#    F2 = "$1"
#   echo "f2 is" $f2
#    f2 += "/"
#    echo "f2b is" $f2
#    f2 += $name
#    echo "f2c is" $f2

#    echo "file is " $name
#    echo "full is " $fullname
#done



## for file in *.wav *.ogg, *.flac; do
    ## name is the fullname (probably with path stripped off)
    ## extention is file extension without the dot
    ## base is the base name withoutthe extension
##   name=${file##*/}
##    extension=${file##*.}
##    base=${name%.$extension}
##    outname=${base}.wav
##    tempname=$base.tmp.$extension

##    echo "converting: " $base

##    mv $name $tempname
##    sox $tempname $outname channels 1 rate 44.1k
# #   rm $tempname
## done

# Patch the sfz files so then can find their sample files,
# which are now .wav
# for file in *.sfz; do
#    echo ""
#    echo "sfz file converting: " $file
#    sed -i 's/\.flac/\.wav/g' $file
#    sed -i 's/\.ogg/\.wav/g' $file
#    sed -i 's/\.WAV/\.wav/g' $file
# done