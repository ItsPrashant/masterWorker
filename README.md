# masterWorker
This repo contains a master and worker program. Master issue computation work to worker program and they together calculate sum of a series.
There are many test modules included in the repo for testing each part of code independently.
The final master program is masterV2.c and worker is worker.c.

Series to be computed is: e^x = 1+ x + (x^2)/2! + (x^3)/3! + .........................

Usage: ./masterV2 --worker_path ./worker --num_workers [validNumber] --wait_mechanism MECHANISM -x [ValueOfX] -n [ValueOfN].
Note : It has been taken care that the max number of workers doesn't the specified number by --num_workers option.
MECHANISM which can be use are-
  1. sequential
  2. select
  3. poll
  4. epoll


