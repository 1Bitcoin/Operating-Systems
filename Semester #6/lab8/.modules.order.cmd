cmd_/home/zhigalkin/lab8/modules.order := {   echo /home/zhigalkin/lab8/lab8.ko; :; } | awk '!x[$$0]++' - > /home/zhigalkin/lab8/modules.order
