OUTPUT=${OUTPUT:-"Unix Makefiles"}
# Bootstrap the build
topdir() {
    (cd "$(pwd)/$(dirname $0)"; pwd)
}
J2DRAGLINE=${J2DRAGLINE:-`topdir`}
BUILDDIR=${J2DRAGLINE}/Build
[ -d ${BUILDDIR} ] || mkdir -p ${BUILDDIR}

# Dependencies
[ -d ${BUILDDIR}/Dependencies ] || mkdir -p ${BUILDDIR}/Dependencies
(cd ${BUILDDIR}/Dependencies; 
 cmake ${J2DRAGLINE}/Dependencies -DJ2DRAGLINE_BINARY_DIR=${BUILDDIR} -G "Unix Makefiles"&&
 make)

# Main
(cd ${BUILDDIR}; cmake -G "$OUTPUT" ${J2DRAGLINE}/Sources -DGTEST_ROOT=${BUILDDIR} -DCMAKE_BUILD_TYPE:STRING=Debug)
echo "(cd ${BUILDDIR}; make)"
