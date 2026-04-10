#!/bin/sh
#configurar porta de rede do host
HOST=`ip route show | head --lines 1 | awk '{print $9}'`
cat $BASE_DIR/../custom-scripts/network-config | sed 's/\[IP-DO-HOST\]/'"$HOST"'/g' > $BASE_DIR/../custom-scripts/S41network-config
cp $BASE_DIR/../custom-scripts/S41network-config $BASE_DIR/target/etc/init.d
chmod +x $BASE_DIR/target/etc/init.d/S41network-config

#configurar o banner de boas vindas
cp $BASE_DIR/../apps/welcome $BASE_DIR/target/usr/bin
cp $BASE_DIR/../custom-scripts/welcome $BASE_DIR/target/etc/init.d/S50welcome
chmod +x $BASE_DIR/target/etc/init.d/S50welcome

#configurar o servidor web
mkdir -p $BASE_DIR/target/var/www/cgi-bin
cp $BASE_DIR/../apps/monitor $BASE_DIR/target/var/www/cgi-bin/monitor
chmod +x $BASE_DIR/target/var/www/cgi-bin/monitor
cp $BASE_DIR/../apps/index.html $BASE_DIR/target/var/www
chmod +x $BASE_DIR/target/var/www/index.html

#criar usuário/grupo não privilegiado para o httpd
grep -q '^httpd:' $BASE_DIR/target/etc/group 2>/dev/null || \
    echo 'httpd:x:1001:' >> $BASE_DIR/target/etc/group

grep -q '^httpd:' $BASE_DIR/target/etc/passwd 2>/dev/null || \
    echo 'httpd:x:1001:1001:httpd:/var/www:/bin/sh' >> $BASE_DIR/target/etc/passwd

chown -R 1001:1001 $BASE_DIR/target/var/www
chmod 755 $BASE_DIR/target/var/www
chmod 755 $BASE_DIR/target/var/www/cgi-bin
chmod 755 $BASE_DIR/target/var/www/cgi-bin/monitor

#iniciar servidor web
cp $BASE_DIR/../custom-scripts/httpd $BASE_DIR/target/etc/init.d/S51httpd
chmod +x $BASE_DIR/target/etc/init.d/S51httpd
