cmd_/home/zhigalkin/lab8/Module.symvers := sed 's/ko$$/o/' /home/zhigalkin/lab8/modules.order | scripts/mod/modpost -m -a   -o /home/zhigalkin/lab8/Module.symvers -e -i Module.symvers   -T -
