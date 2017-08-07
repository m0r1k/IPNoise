#!/bin/sh

## set -x

TRANSPORT="tcp"

. notebook_setup_1

ip neigh replace 2210:0:349e:f515:36b5:ff1f:e5e7:118a dev tcp0 lladdr s:127.0.0.1:25500

