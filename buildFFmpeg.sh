git clone https://github.com/FFmpeg/FFmpeg.git --branch n4.2 --single-branch
filepath=$(realpath $0)
cwd=$(dirname $filepath)
echo $cwd
cd FFmpeg
./configure --prefix=$cwd/tempFFmpeg --enable-gpl
make install -j8
cd $cwd/tempFFmpeg/lib
mkdir $cwd/fluorender/ffmpeg/lib/Linux
mv *.a $cwd/fluorender/ffmpeg/lib/Linux
cd $cwd
rm -rf tempFFmpeg
