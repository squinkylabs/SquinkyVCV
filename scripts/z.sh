# This script assumes that your wish the convert the
# files in your current directory. As such it takes no arguments on
# the command line.
# Script relies on SOX being installed on your computer and in your path.

# terminate script of first error
set -e
# First find all audio files and converte them to 44k mono wav files
shopt -s nullglob
OIFS="$IFS"
IFS=$'\n'

echo "script invoked on: " $1
echo "arg is "  $1/*.flac


for file in $1/*.flac $1/*.wav $1/*.ogg; do
    name=${file##*/}
    extension=${file##*.}
    base=${name%.$extension}

    echo "in find loop file=$file"
#    echo "base=" $base
#    fullName="$1/${name}"
#    tempName="$1/$base.tmp.$extension"
#    outName="$1/$base.wav"
#    echo "fullname=" $fullName
#    echo "tempname=" $tempName
#    echo "out=     " $outName
#    echo ""
done

IFS="$OIFS"

