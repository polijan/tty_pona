#!/usr/bin/env bash

# 󱥂󱤞.bash
# 󱥧󱤑󱦐Polijan󱦑
#
# 󱤎󱤻󱥁󱤧󱤈󱥠󱤉󱥂󱥍󱥬󱥔󱤬󱤞

while ! read -r -s -n 1 -t 0.01; do
   kule=$((1 + RANDOM % 255))
   nimi=$((1 + RANDOM % 137))
   printf -v nanpa_nimi '%02X' "$nimi"
   printf '\033[38;5;%dm%b' $kule "\UF19${nanpa_nimi}"
done
printf '\033[m\n'
