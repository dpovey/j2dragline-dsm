# Bootstrap the build
topdir() {
    (cd "$(pwd)/$(dirname $0)"; pwd)
}
J2DRAGLINE=${J2DRAGLINE:-`topdir`}
BUILDDIR=${J2DRAGLINE}/Build
[ -d ${BUILDDIR} ] || mkdir -p ${BUILDDIR}
(cd ${BUILDDIR}; cmake ${J2DRAGLINE}/Sources)
echo "Run make from ${BUILDDIR}"
