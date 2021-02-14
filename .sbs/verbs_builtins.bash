#!/bin/bash

build() {
    source .sbs/sbs.bash
}

clean() {
    echo "Cleaning objects and temporary files"
    if [[ -f ${PROJECT_FILE} ]]; then
        source ${PROJECT_FILE}
    else
        echo "Project file not found!"
        exit 1
    fi
    BUILD_DIR=${BUILD_DIR:-"${PROJECT_DIR}/build/"}
    _RBDIR=${_RBDIR:-"${BUILD_DIR}/"}
    [[ -d "${_RBDIR}/debug/obj" ]] && rm -rf "${_RBDIR}/debug/obj"
    [[ -d "${_RBDIR}/release/obj" ]] && rm -rf "${_RBDIR}/release/obj"
    [[ -d ".sbs/tmp" ]] && rm -rf ".sbs/tmp"
    echo "Cleaned!"
}

remove_update_files() {
    echo "Removing temporary file..."
    if [[ $(basename ${PWD}) = "${TMP_DIR}" ]]; then
        cd ..
    fi
    rm -rf ${TMP_DIR}
    if [[ $? -ne 0 ]]; then
        echo "Problem while trying to delete temporary files!"
        echo "Try deleting '${TMP_DIR}' by yourself, it should be deleted automatically after reboot"
        exit 1
    fi
}

update() {
    TMP_DIR=$(mktemp -d)
    cd ${TMP_DIR}

    if [[ ! $(command -v jq) ]]; then
        echo "Cannot find command jq, please install it first"
        exit 1
    fi

    GITHUB_API_LATEST="https://api.github.com/repos/TheCoderCrab/ShellBuildSystem/releases/latest"

    echo "Fetching update meta-data from: \"${GITHUB_API_LATEST}\"..."

    if [[ $(command -v wget) ]]; then
        TAG_NAME=$(wget -O - -q ${GITHUB_API_LATEST} | jq -r ".tag_name")
    elif [[ $(command -v curl) ]]; then
        TAG_NAME=$(curl -s ${GITHUB_API_LATEST} | jq -r ".tag_name")
    else
        echo "Please install wget or curl before trying to update"
        cd ..
        remove_update_files
        exit 1
    fi

    
    if [[ -f ../.sbs/sbs.version ]]; then
        CURRENT_VERSION=$(cat ../.sbs/sbs.version)
    else
        CURRENT_VERSION="unknown"
    fi

    echo "Latest version: ${TAG_NAME}"
    echo "Current version: ${CURRENT_VERSION:-"unknown"}"

    if [[ ${CURRENT_VERSION} != ${TAG_NAME} ]]; then
        echo "Installing new update..."
    else
        echo "Latest update already installed"
        exit 0
    fi

    UPDATE_URL="https://github.com/TheCoderCrab/ShellBuildSystem/archive/${TAG_NAME}.tar.gz"
    
    echo "Downloading update..."
    
    if [[ $(command -v wget) ]]; then
        CMD="wget -q"
    elif [[ $(command -v curl) ]]; then
        CMD="curl -s -L -o ${TAG_NAME}.tar.gz"
    fi
    ${CMD} ${UPDATE_URL}
    if [[ $? -ne 0 ]]; then
        echo "Problem while downloading the update files!"
        remove_update_files
        exit 1
    fi
    echo "Extracting files..."
    tar -xzf ${TAG_NAME}.tar.gz
    if [[ $? -ne 0 ]]; then
        echo "Problem while extracting the update files!"
        remove_update_files
        exit 1
    fi
    echo "Copying new files to .sbs directory..."
    \cp -r ShellBuildSystem*/.sbs/* ../.sbs
    if [[ $? -ne 0 ]]; then
        echo "Problem while copying the update files!"
        remove_update_files
        exit 1
    fi
    cd ..
    remove_update_files
    echo ${TAG_NAME} > .sbs/sbs.version
    echo "Updated sbs!"
}
