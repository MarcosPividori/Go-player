#Install Gogui.
wget -O gogui-1.4.9.zip http://sourceforge.net/projects/gogui/files/gogui/1.4.9/gogui-1.4.9.zip/download
unzip gogui-1.4.9.zip
rm gogui-1.4.9.zip
cd gogui-1.4.9; \
sudo ./install.sh
rm -rf gogui-1.4.9

#Install GnuGo.	
wget -O gnugo-3.8.tar.gz http://ftp.gnu.org/gnu/gnugo/gnugo-3.8.tar.gz
tar -zxvf gnugo-3.8.tar.gz
rm gnugo-3.8.tar.gz
cd gnugo-3.8;\
./configure;\
make
sudo make install
rm -rf gnugo-3.8
