# VBIT Pi automatic start #

Make a start up script by editing a new file<br />
sudo nano /etc/init.d/vbit.sh<br />
and put this code in it

```
#! /bin/sh
# /etc/init.d/vbit.sh
#

# Some things that run always
#

# Carry out specific functions when asked to by the system
case "$1" in
  start)
    cd /home/pi/vbit-pi
    ./setup.sh
    sudo ./vbit &
    echo "Starting script vbit"
    echo "Could do more here"
    ;;
  stop)
    echo "Stopping script vbit"
    echo "Could do more here"
    ;;
  *)
    echo "Usage: /etc/init.d/vbit {start|stop}"
    exit 1
    ;;
esac

exit 0
```

make it executable<br />
sudo chmod 755 /etc/init.d/vbit.sh<br />
and register it <br />
$ sudo update-rc.d vbit.sh defaults