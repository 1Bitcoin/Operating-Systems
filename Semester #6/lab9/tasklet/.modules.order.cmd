cmd_/home/zhigalkin/tasklet/modules.order := {   echo /home/zhigalkin/tasklet/tasklet.ko; :; } | awk '!x[$$0]++' - > /home/zhigalkin/tasklet/modules.order
