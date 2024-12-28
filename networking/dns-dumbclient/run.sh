#!/usr/bin/bash

gcc main.c -o /tmp/dns-dumbclient && /tmp/dns-dumbclient $1
