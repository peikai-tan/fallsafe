script_dir=$(dirname "$0")

while true; 
do 
    temp=$(/opt/vc/bin/vcgencmd measure_temp)
    volts=$(vcgencmd measure_volts)
    throttle=$(/opt/vc/bin/vcgencmd get_throttled)
    throttle_string=$(bash $script_dir/throttle-decoded.sh)
    printf "$temp $volts $throttle $throttle_string\n"
    sleep 10; 
done
