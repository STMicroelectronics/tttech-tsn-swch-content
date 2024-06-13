#!/bin/sh

### BEGIN INIT INFO
# Provides:          deipce
# Short-Description: Load DE-IP Core Edge drivers
# Required-Start:    $local_fs
# Required-Stop:     $local_fs
# X-Start-Before:    networking
# Default-Start:     2 3 4 5
# Default-Stop
### END INIT INFO

pio_sysfs_name="c00f0f00.gpio"
fpga_eth_reset=""

# Determine first GPIO number of GPIO device.
# Sets variable find_gpio to GPIO number.
# Returns 0 on success.
# Usage: find_gpio SYSFS_GPIO_DEVNAME
find_gpio()
{
	name=$1
	gpio_sysfs_path=$(find /sys/devices -name "$name" | head -n 1)
	for i in "$gpio_sysfs_path/gpio"/* ; do
		test -e "$i" || continue
		find_gpio=${i##*/gpiochip}
		return 0
	done
	return 1
}

# Configure GPIO to given direction and given value in case of output.
# Usage: set_gpio GPIONUM {in|out} [VALUE]
set_gpio()
{
	num=$1
	dir=$2
	value=$3

	test -d /sys/class/gpio/gpio$num ||
		echo "$num" > /sys/class/gpio/export
	echo "$dir" > /sys/class/gpio/gpio$num/direction
	test "$dir" != out || echo "$value" >/sys/class/gpio/gpio$num/value
}

start()
{
	echo "Loading drivers..."
	if find_gpio $pio_sysfs_name ; then
		set_gpio $find_gpio out 0
		sleep 1
		set_gpio $find_gpio out 1
	fi
}

stop()
{
	echo "Unloading drivers..."
	loaded="$(lsmod | awk '{ print $1 }')"
	echo "$loaded" | grep -q '^deipce$' && rmmod deipce
	echo "$loaded" | grep -q '^flx_eth_mdio$' && rmmod flx_eth_mdio

	if find_gpio $pio_sysfs_name ; then
		if test -d /sys/class/gpio/gpio$find_gpio ; then
			echo $find_gpio > /sys/class/gpio/unexport
		fi
	fi
	echo "$loaded" | grep -q '^flx_pio$' && rmmod flx_pio
}

case "$1" in
	start|restart|force-reload)
		echo "Configuring FPGA ..."
		start
		;;
	stop)
		stop
		;;
esac

exit 0
