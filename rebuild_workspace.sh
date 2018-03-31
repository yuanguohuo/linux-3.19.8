#! /bin/bash

# 1. generate ctags, which support OmniCppComplete(also works for C):
#
# -f .tags: the name of the generated tagfile, default is tags;
# --c-kinds=+pl
# --c++-kinds=+pl, p: add function prototypes. it is closed by default; 
#		   l: generate tag information for local variables;
#
# --fields=+iaS,   i: add inheritance information; 
#		   a: add access control information;
# 		   S: add function signature; 
#
# --extra=+q, add extra information, such as class modifier;

#use the exuberant ctags I installed, instead of the ctags of mac os;
#CTAGS=/usr/local/exuberant-ctags/bin/ctags
#For Linux, not needed.
CTAGS=ctags
$CTAGS -f .tags -R --languages=c,c++ --c-kinds=+p --c++-kinds=+p --fields=+iaS --extra=+q *

# 2. generate the reference data base for cscope:
find -L . -type f -exec realpath {} \; | grep \
    -e "\.c$"            \
    -e "\.h$"            \
    -e "\.cc$"           \
    -e "\.hh$"           \
    -e "\.cxx$"          \
    -e "\.hxx$"          \
    -e "\.c++$"          \
    -e "\.h++$"          \
    -e "\.cpp$"          \
    -e "\.hpp$"          \
    -e "\.java$"         \
    -e "\.Java$"         \
    -e "\.lua$"          \
    > .cscope.files

#mac os doesn't have cscope, use the one I installed;
#CSCOPE=/usr/local/cscope/bin/cscope
#For Linux, not needed.
CSCOPE=cscope
$CSCOPE -bq -i .cscope.files -f .cscope.out

# 3. generate tags for lookup plugin: 
#
# Lookup plugin is used to serach files. To search files, it takes tags file as
# input. We have already generated a tags file for ctags tool, but that file
# contains numerous functions, variables, macros and so on, besides file names.
# Since only file names in tags file are useful for lookup plugin, we generate a
# new tags file for it, only containing file names. This tags file is much
# samller, so it makes lookup faster.
# The new tags file is named 'filenametags'. See .vimrc where this file is
# loaded to vim.

echo -e "!_TAG_FILE_SORTED\t2\t/2=foldcase/" > .filenametags
TMPFILE="/tmp/rebuild-ws-filenametags-`date +%s`"

for f in `find -L . -not -regex '.*\.\(png\|gif\|svn-base\|d\|o\|so\)' -type f | grep -v "/\.svn/"` ; do
    base=`basename $f`
    echo "$base	$f	1" >> $TMPFILE
done

cat $TMPFILE | sort -f >> .filenametags
