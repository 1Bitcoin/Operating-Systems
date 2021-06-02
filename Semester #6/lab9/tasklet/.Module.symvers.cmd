cmd_/home/zhigalkin/tasklet/Module.symvers := sed 's/ko$$/o/' /home/zhigalkin/tasklet/modules.order | scripts/mod/modpost -m -a   -o /home/zhigalkin/tasklet/Module.symvers -e -i Module.symvers   -T -
