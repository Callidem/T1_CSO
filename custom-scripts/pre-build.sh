#!/bin/sh

HOST=`ip route show | head --lines 1 | awk '{print $9}'`
cat $BASE_DIR/../custom-scripts/network-config | sed 's/\[IP-DO-HOST\]/'"$HOST"'/g' > $BASE_DIR/../custom-scripts/S41network-config
cp $BASE_DIR/../custom-scripts/S41network-config $BASE_DIR/target/etc/init.d
chmod +x $BASE_DIR/target/etc/init.d/S41network-config

cp $BASE_DIR/../apps/welcome $BASE_DIR/target/usr/bin
cp $BASE_DIR/../custom-scripts/welcome $BASE_DIR/target/etc/init.d/S50welcome
chmod +x $BASE_DIR/target/etc/init.d/S50welcome

mkdir -p $BASE_DIR/target/var/www/cgi-bin
cp $BASE_DIR/../apps/monitor $BASE_DIR/target/var/www/cgi-bin/monitor
chmod +x $BASE_DIR/target/var/www/cgi-bin/monitor

cat > /var/www/index.html << 'EOF'
<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="refresh" content="0; url=/cgi-bin/monitor">
    <title>Redirecionando...</title>
</head>
<body>
    <p>Redirecionando para o monitor do sistema...</p>
</body>
</html>
EOF

cp $BASE_DIR/../custom-scripts/httpd $BASE_DIR/target/etc/init.d/S51httpd
chmod +x $BASE_DIR/target/etc/init.d/S51httpd
