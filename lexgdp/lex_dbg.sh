#!/bin/bash
# use testnet settings,  if you need mainnet,  use ~/.lex/finto.pid file instead
lex_pid=$(<~/.lexthumcore/testnet3/lex.pid)
sudo gdb -batch -ex "source debug.gdb" lexd ${lex_pid}
