#!/bin/bash

# Uncomment for additional debug info
# SBS_DEBUG=1


PROJECT_DIR=${PROJECT_DIR:-$PWD}
SBSRC=${SBSRC:-"${PROJECT_DIR}/sbsrc"}
SBS_DIR=${SBS_DIR:-"${PROJECT_DIR}/.sbs"}
PROJECT_FILE=${PROJECT_FILE:-"${PROJECT_DIR}/sbs.project"}
TMP_DIR=${TMP_DIR:-"${SBS_DIR}/tmp/"}
SUB_PROJECT_RET="${TMP_DIR}/sub_proj_ret"
LAST_BUILD_FILE="${SBS_DIR}/last_build"
if [[ -f ${LAST_BUILD_FILE} ]]; then
    LAST_BUILD_TIME=$(cat ${LAST_BUILD_FILE})
else
    LAST_BUILD_TIME=0
fi


cd ${PROJECT_DIR}

[[ -f ${SBSRC} ]] && source ${SBSRC}

mkdir -p ${TMP_DIR}

source ${SBS_DIR}/exit_code.bash
source ${SBS_DIR}/term.bash
source ${SBS_DIR}/colors.bash

debug "Sbs is in debug mode"
debug "Last build: ${LAST_BUILD_TIME}"


_IS_SUB_PROJECT=${_IS_SUB_PROJECT:-0}

if [[ ${_IS_SUB_PROJECT} -eq 0 ]]; then
    print "${bold}${fg_green}Started sbs"
fi

if [[ -f ${PROJECT_FILE} ]]; then
    print "${bold}${fg_green}Reading project file..."
    source ${PROJECT_FILE}
else
    error "${underline}$(basename ${PROJECT_FILE})${end_underline} file not found!"
    exit ${exit_error}
fi

# Set default values
CC=${CC:-"/bin/gcc"}
CXX=${CXX:-"/bin/g++"}
AS=${AS:-"/bin/nasm"}
LINKER=${LINKER:-"/bin/g++"}
STATIC_LINKER=${STATIC_LINKER:-"/bin/ar"}
CFLAGS=${CFLAGS:-""}
CXXFLAGS=${CXXFLAGS:-""}
CCFLAGS=${CCFLAGS:-""}
ASFLAGS=${ASFLAGS:-""}
LINKFLAGS=${LINKFLAGS:-""}
CEXTENSIONS=${CEXTENSIONS:-".c"}
CXXEXTENSIONS=${CXXEXTENSIONS:-".cc .cxx .cpp .c++"}
ASEXTENSIONS=${ASEXTENSIONS:-".s .asm .as"}
SRC=${SRC:-"${PROJECT_DIR}/src/"}
ADDITIONAL_SOURCES=${ADDITIONAL_SOURCES:-""}
INCLUDE=${INCLUDE:-"${PROJECT_DIR}/include"}
PUBLIC_INCLUDE=${PUBLIC_INCLUDE:-""}
TARGET=${TARGET:-"out"}
TYPE=${TYPE:-"exec"}
BUILD_TYPE=${BUILD_TYPE:-"debug"}
BUILD_DIR=${BUILD_DIR:-"${PROJECT_DIR}/build/"}
SUB_PROJECTS=${SUB_PROJECTS:-""}
LIBRARIES=${LIBRARIES:-""}
LIB_DIRS=${LIB_DIRS:-""}

print "${bold}${fg_green}Preparing to build target: \"${fg_blue}${TARGET}${fg_green}\"..."

# Define extended variables
_RBDIR=${_RBDIR:-"${BUILD_DIR}/${BUILD_TYPE}/"}
_BDIR=${_BDIR:-"${_RBDIR}/obj"}

mkdir -p ${_RBDIR}
mkdir -p ${_BDIR}

# Convert some variables into lists
export IFS=${IFS:-" "}
read -r -a _CEXT   <<< "${CEXTENSIONS}"
read -r -a _CXXEXT <<< "${CXXEXTENSIONS}"
read -r -a _ASEXT  <<< "${ASEXTENSIONS}"
read -r -a _SRC    <<< "${SRC}"
read -r -a _ASRC   <<< "${ADDITIONAL_SOURCES}"
read -r -a _INC    <<< "${INCLUDE}"
read -r -a _PINC   <<< "${PUBLIC_INCLUDE}"
read -r -a _SPR    <<< "${SUB_PROJECTS}"
read -r -a _LIBS   <<< "${LIBRARIES}"
read -r -a _LDIRS  <<< "${LIB_DIRS}"


if [[ ${TYPE} = "exec" ]]; then
    OUT_NAME="${TARGET}"
elif [[ ${TYPE} = "slib" ]]; then
    OUT_NAME="lib${TARGET}.a"
elif [[ ${TYPE} = "dlib" ]]; then
    CCFLAGS+=" -fPIC"
    OUT_NAME="lib${TARGET}.so"
else
    error "Invalid build type: ${TYPE}"
    exit ${exit_error}
fi

OUT="${_RBDIR}/${OUT_NAME}"

# All the sources

C_SOURCES=()
CXX_SOURCES=()
AS_SOURCES=()

for SOURCE_DIR in ${_SRC[@]}; do
    mkdir -p ${SOURCE_DIR}
    debug "Iterating through source dir: ${SOURCE_DIR}"
    for CEXT in ${_CEXT[@]}; do
        debug "Iterating through C extension: ${CEXT}"
        for SOURCE in $(find ${SOURCE_DIR} -type f -name "*${CEXT}"); do
            debug "Found C source file: ${SOURCE}"
            C_SOURCES+=("${SOURCE}")
        done
    done
    for CXXEXT in ${_CXXEXT[@]}; do
        debug "Iterating through C++ extension: ${CXXEXT}"
        for SOURCE in $(find ${SOURCE_DIR} -type f -name "*${CXXEXT}"); do
            debug "Found C++ source file: ${SOURCE}"
            CXX_SOURCES+=("${SOURCE}")
        done
    done
    for ASEXT in ${_ASEXT[@]}; do
        debug "Iterating through AS extension: ${ASEXT}"
        for SOURCE in $(find ${SOURCE_DIR} -type f -name "*${ASEXT}"); do
            debug "Found AS source file: ${SOURCE}"
            AS_SOURCES+=("${SOURCE}")
        done
    done
done

debug "Source file count: C: ${#C_SOURCES[@]}, C++: ${#CXX_SOURCES[@]}, AS: ${#AS_SOURCES[@]}, Add: ${#_ASRC[@]}"

if [[ ${#_ASRC[@]} -eq 0 && ${#C_SOURCES[@]} -eq 0 &&\
      ${#CXX_SOURCES[@]} -eq 0 && ${#AS_SOURCES[@]} -eq 0 ]]; then
    error "No source file found!"
    exit ${exit_error}
fi

# Check if compilers, assembler and linker exists

if [[ $(command -v ${CC}) ]]; then
    print "${bold}${fg_green}C Compiler: ${CC} found"
else
    error "C Compiler: ${CC} not found!"
    exit ${exit_error}
fi

if [[ $(command -v ${CXX}) ]]; then
    print "${bold}${fg_green}C++ Compiler: ${CXX} found"
else
    error "C++ Compiler: ${CXX} not found!"
    exit ${exit_error}

fi

if [[ $(command -v ${AS}) ]]; then
    print "${bold}${fg_green}Assembler: ${AS} found"
else
    error "Assembler: ${AS} not found!"
    exit ${exit_error}
fi

# Build all subprojects

for PROJ in ${_SPR[@]}; do
    __PROJECT_NAME=$(basename $PROJ)
    if [[ ${PROJ:0:1} = '/' ]]; then
        debug "Subproject ${__PROJECT_NAME} is defined relative to root"
        __SUB_PROJ_DIR=${PROJ}
    else
        debug "Subproject ${__PROJECT_NAME} is defined relative to $(basename ${PROJECT_DIR})"
        __SUB_PROJ_DIR="${PROJECT_DIR}/${PROJ}"
    fi
    print "${fg_green}${bold}Building subproject \"${fg_blue}${__PROJECT_NAME}${fg_green}\""
    env -i PATH="${PATH}" TERM="${TERM}" PROJECT_DIR="${__SUB_PROJ_DIR}" SBS_DIR="${SBS_DIR}"\
    PROJECT_FILE="${__SUB_PROJ_DIR}/$(basename ${PROJECT_FILE})" SBSRC=${SBSRC}               \
    CC="${CC}" CXX="${CXX}" AS="${AS}" LINKER="${LINKER}" CEXTENSIONS="${CEXTENSIONS}"         \
    CXXEXTENSIONS="${CXXEXTENSIONS}" ASEXTENSIONS="${ASEXTENSIONS}" BUILD_TYPE="${BUILD_TYPE}"  \
    BUILD_DIR="${BUILD_DIR}/${__PROJECT_NAME}" _IS_SUB_PROJECT=1 CFLAGS="${CFLAGS}"              \
    CXXFLAGS="${CXXFLAGS}" CCFLAGS="${CCFLAGS}" ASFLAGS="${ASFLAGS}"                              \
    bash ${SBS_DIR}/sbs.bash
    if [[ $? -ne 0 ]]; then
        error "Subproject ${__PROJECT_NAME} didn't execute properly"
        exit ${exit_error}
    fi
    source ${SUB_PROJECT_RET}
    if [[ ${_SPR_TYPE} != "exec" ]]; then # if lib
        print "${bold}${fg_green}Adding subproject ${fg_blue}${__PROJECT_NAME}${fg_green} as a library"
        _PINC+=("${_SPR_INCLUDE[@]}")
        _LIBS+=("${_SPR_TARGET}")
        _LDIRS+=("${_SPR_BUILD_DIR}")
    fi
done

# Build self

INCLUDE_FLAGS=""
AS_INCLUDE_FLAGS=""
LIB_DIRS_FLAGS=""
LIB_FLAGS=""
OBJECTS=""

for INC in ${_INC[@]} ${_PINC[@]}; do
    INCLUDE_FLAGS+=" -I${INC}"
    AS_INCLUDE_FLAGS+=" -i${INC}"
done

for LIB_DIR in ${_LDIRS[@]}; do
    LIB_DIRS_FLAGS+=" -L${LIB_DIR}"
done

for LIB in ${_LIBS[@]}; do
    LIB_FLAGS+=" -l${LIB}"
done

debug " Include flags: \"${INCLUDE_FLAGS}\""

RELINK=0

compile() {
    if [[ ${#} -ne 1 ]]; then
        error "compile got invalid arguments"
        exit -1
    fi
    SOURCE_OUT="${_BDIR}/${SOURCE//\//$'_'}.o"
    SOURCE_EXT="${SOURCE##*.}"
    OBJECTS+=" ${SOURCE_OUT}"
    SOURCE_MODIF=$(stat -c %Y ${SOURCE})
    if [[ ! -f ${SOURCE_OUT} || ${SOURCE_MODIF} -gt ${LAST_BUILD_TIME} ]]; then
        RELINK=1
        if [[ ${_CEXT[@]} =~ "${SOURCE_EXT}" ]]; then
            print " ${bold}${fg_yellow}=> ${fg_green}Compiling $(basename ${SOURCE})"
            CMD="${CC} -c ${INCLUDE_FLAGS} ${CFLAGS} ${CCFLAGS} ${SOURCE} -o ${SOURCE_OUT}"
            ${CMD}
            if [[ $? -ne 0 ]]; then
                error "Failed to compile: ${SOURCE}"
                error "While executing: "${CMD}""
                exit ${exit_error}
            fi
        elif [[ ${_CXXEXT[@]} =~ "${SOURCE_EXT}" ]]; then
            print " ${bold}${fg_yellow}=> ${fg_green}Compiling $(basename ${SOURCE})"
            CMD="${CXX} -c ${INCLUDE_FLAGS} ${CXXFLAGS} ${CCFLAGS} ${SOURCE} -o ${SOURCE_OUT}"
            ${CMD}
            if [[ $? -ne 0 ]]; then
                error "Failed to compile: ${SOURCE}"
                error "While executing: "${CMD}""
                exit ${exit_error}
            fi
        elif [[ ${_ASEXT[@]} =~ "${SOURCE_EXT}" ]]; then
            print " ${bold}${fg_yellow}=> ${fg_green}Assembling $(basename ${SOURCE})"
            CMD="${AS} ${AS_INCLUDE_FLAGS} ${ASFLAGS} ${SOURCE} -o ${SOURCE_OUT}"
            ${CMD}
            if [[ $? -ne 0 ]]; then
                error "Failed to assemble: ${SOURCE}"
                error "While executing: "${CMD}""
                exit ${exit_error}
            fi
        fi
    else
        print " ${bold}${fg_blue}=> ${fg_green} Skipping $(basename ${SOURCE}), already up to date"
    fi
}

for SOURCE in ${_ASRC[@]}; do
    compile ${SOURCE}
done

for SOURCE in ${C_SOURCES[@]}; do
    compile ${SOURCE}
done

for SOURCE in ${CXX_SOURCES[@]}; do
    compile ${SOURCE}
done

for SOURCE in ${AS_SOURCES[@]}; do
    compile ${SOURCE}
done

if [[ ${RELINK} -ne 0 || ! -f ${OUT} ]]; then
    if [[ ${TYPE} = "exec" ]]; then
        print " ${bold}${fg_yellow}=> ${fg_green}Linking executable: ${fg_blue}$(basename ${OUT})${fg_green}..."
        CMD="${LINKER} ${LINKFLAGS} ${OBJECTS} ${LIB_DIRS_FLAGS} ${LIB_FLAGS} -o ${OUT}"
    elif [[ ${TYPE} = "slib" ]]; then
        print " ${bold}${fg_yellow}=> ${fg_green}Generating static library: ${fg_blue}$(basename ${OUT})${fg_green}..."
        CMD="${STATIC_LINKER} crf "${_RBDIR}/lib${TARGET}.a" ${OBJECTS}"
    elif [[ ${TYPE} = "dlib" ]]; then
        print " ${bold}${fg_yellow}=> ${fg_green}Linking shared library: ${fg_blue}$(basename ${OUT})${fg_green}..."
        CMD="${LINKER} -shared ${LINKFLAGS} ${OBJECTS} ${LIB_DIRS_FLAGS} ${LIB_FLAGS} -o ${OUT}"
    fi

    ${CMD}
    if [[ $? -ne 0 ]]; then
        error "Linking failed!"
        error "While executing: ${CMD}"
        exit ${exit_error}
    fi
else
    print " ${bold}${fg_blue}=> ${fg_green} Skipping ${OUT}, already up to date"
fi


if [[ ${_IS_SUB_PROJECT} -eq 1 ]]; then
    ABSOLUTE_INCLUDE=""
    for PINC in ${_PINC[@]}; do
        if [[ ${PINC:0:1} = "/" ]]; then
            ABSOLUTE_INCLUDE+=" ${PINC}"
        else
            ABSOLUTE_INCLUDE+=" ${PWD}/${PINC}"
        fi
    done
    printf "_SPR_TYPE=${TYPE}\n_SPR_INCLUDE=(${ABSOLUTE_INCLUDE})\n_SPR_TARGET=${TARGET}\n_SPR_BUILD_DIR=${_RBDIR}\n" > ${SUB_PROJECT_RET}
else
    echo $(date +%s) > ${LAST_BUILD_FILE}
    rm -rf ${TMP_DIR}
fi

print "${bold}${fg_green}Done building project: ${TARGET}" 
