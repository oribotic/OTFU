# OTFU
Scripts and code for running Oribotics the Future Unfolds


Quick install notes:

# Dependencies 

mysql gcc apache php

sudo apt-get install mysql-server mysql-client default-libmysqlclient-dev apache2 apache2-doc gcc php php-mysql

sudo mysql_secure_installation

To install: 
Copy the files, SFTP protocol is easiest. Port 22: 192.168.0.50
Run the following commands as root from within the oribotics-setup-code directory.

<VirtualHost *:80>
        #ServerName www.example.com

        ServerAdmin hello@matthewgardiner.net
        DocumentRoot /srv/oribot/
        <Directory /srv/oribot/>
                Options +MultiViews +FollowSymLinks +Indexes
                AllowOverride All
                Require all granted
                #Allow from all
                AddType application/x-httpd-php .php
        </Directory>
        LogLevel info
        ErrorLog ${APACHE_LOG_DIR}/error.log
        CustomLog ${APACHE_LOG_DIR}/access.log combined

</VirtualHost>


# 1. install the watchdog script
cp safe_ori.sh /sbin/safe_ori.sh
chmod +x /sbin/safe_ori.sh

#1.2 install into startup rc.local

sudo pico /etc/rc.local

- add lines before exit 0

sudo /home/pi/Oribot/startup_ori_sql.sh &
sudo /sbin/safe_ori.sh

# 2. install the web interface, and chown the files for the webservers
cp -R setup /srv/oribot
chown -R www-data:www-data /srv/oribot/templates_c
chown www-data:www-data /srv/oribot/templates_c/*

- symbolic link to oribot directory for bot.sql

ln -s /srv/oribot/bot.sql /home/pi/Oribot/bot.sql

# 3. update the database

CREATE USER 'oribot'@'%' IDENTIFIED BY 'oribot';
GRANT ALL PRIVILEGES ON *.* TO 'oribot'@'%' WITH GRANT OPTION;

mysql -u root -D oribotics --password=oribot < oribotics.sql

# 4. copy the ori binary to sbin, change to executable
cp Oribot/ori /sbin/ori
chmod +x /sbin/ori

# 5. copy the source to the local directory in case we need to rebuild it later
cp -R Oribot /home/ray/Oribot_2013


# afterwards find the two ori process IDs
ps -ax | grep ori
# use the process ID to kill them
kill {ID}
# restart the watchdog or directly start the code from sbin
/sbin/safe_ori &

#OR for testing

/sbin/ori -t -v 



###### MYSQL PROBLEMS

if the database does not seem to be loading
try:
root@ts7800:root# /etc/init.d/mysql start

If you get this error
/etc/init.d/mysql: ERROR: The partition with /var/lib/mysql is too full!

then
ls -al /var/lib/mysql

remove the file /var/lib/mysql/ibdata*
rm /var/lib/mysql/ibdata*

