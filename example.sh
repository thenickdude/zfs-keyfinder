#!/usr/bin/env bash

searchvolume=/dev/sda
dataset=tank/my-dataset

pv "${searchvolume}" | ./zfs-keyfinder | unbuffer -p cut -d' ' -f2 |
while read -r candidate; do
    zfs load-key -n -L file://<(echo -n "${candidate}" | xxd -r -p) "${dataset}" \
      && echo "Found working key: ${candidate}" \
      && break
done