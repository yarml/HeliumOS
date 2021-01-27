#!/bin/bash

print() {
    echo "${reset} ${@} ${reset}"
}

debug() {
    if [[ -v SBS_DEBUG ]]; then
        print "${bold}${fg_magenta}Debug${reset}: ${@}" 
    fi
}

warn() {
    print "${bold}${fg_yellow}Warning${reset}: " ${@}
}

error() {
    print "${bold}${fg_red}Error${reset}: " ${@} 
}