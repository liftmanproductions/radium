#!/bin/sh

PYTHONEXE=$1
MOC=$2
UIC=$3

set -e

echo
echo "Checking dependencies: "
which sed
which $PYTHONEXE
#which guile-1.8


if $1 -c "import sys ; sys.exit(sys.version[:1] == \"2\")" ; then
    echo "Only Python 2 is supported:"
    $1 --version
    echo
    exit -1
fi

if ! which $MOC ; then
    echo "Can not find moc. Make sure QTDIR and/or MOC is set correctly in the Makefile".
    echo
    exit -1
fi

if ! which $UIC ; then
    echo "Can not find uic. Make sure QTDIR and/or UIC set correctly in the Makefile".
    echo
    exit -1
fi

if $MOC -v 2>&1 |grep Qt\ 3 ; then
    echo $MOC "is for QT3. Need moc for QT4. Make sure MOC is set correctly in the Makefile."
    echo
    exit -1
fi

if $UIC -v 2>&1 |grep Qt\ 3 ; then
    echo $UIC "is for QT3. Need uic for QT4. Make sure UIC is set correctly in the Makefile."
    echo
    exit -1
fi


if grep -e "\ \*" api/protos.conf ; then
    echo "The above line in api/protos.conf is wrongly formatted. Must use \"<type>*\", not \"<type> *\""
    echo
    exit -1
fi



echo "#include <X11/Xaw/Scrollbar.h>" >temp$$.c
echo "main(){return 0;}" >>temp$$.c
echo >>temp$$.c
if ! gcc temp$$.c -lXaw ; then
    echo "Might be missing libXaw-devel"
    echo
    rm temp$$.c
    exit -1
fi
rm temp$$.c


if [ ! -f bin/packages/deletemetorebuild ] ; then
    echo
    echo "Packages not build. First run 'make packages'"
    echo
    exit -1
fi

echo "All seems good"
echo

