#!/bin/bash

output_file=out.c
input_files=

function usage()
{
    echo "Usage: $0 [-o output.c] input.c..." 1>&2
    exit 1
}

while [ $# -gt 0 ] ; do
    case "$1" in
    -o|--output) output_file="$2" ; shift ;;
    --output=*) output_file="${2#*=}" ;;
    --help) usage ;;
    -*) usage ;;
    *) input_files="$input_files $1" ;;
    esac
    shift
done
[ -n "$input_files" ] || usage

# echo "output_file=\"$output_file\""
# echo "input_files=\"$input_files\""
# exit 0

_host_os=$(uname -s)
case "$_host_os" in
Darwin)
    function list_symbols()
    {
        nm "$1" | awk '$2=="T"{print substr($3,2,length($3)-1)}'
    }
    ;;
Linux)
    function list_symbols()
    {
        nm "$1" | awk '$2=="T"{print $3}'
    }
    ;;
esac

t1=t1
t2=t2
trap "/bin/rm -f $t1 $t2" 0 1 11 13 15
/bin/rm -f $t1 $t2

ntests=0
for file in $input_files ; do
    case "$file" in
    c_unit_*.o) continue ;;
    esac
    base=$(basename "$file" .o)
    test_funcs=$(list_symbols $file | egrep '^test(_[a-z][a-zA-Z0-9_]*|[A-Z][a-zA-Z0-9]*$)')
    setup_func=$(list_symbols $file | egrep '^(set_up|setUp)$')
    teardown_func=$(list_symbols $file | egrep '^(tear_down|tearDown)$')
    for test_func in $test_funcs ; do
        echo "extern void $test_func(void); /* $file */" >> $t1
        if [ -n "$setup_func" -o -n "$teardown_func" ] ; then
            echo "    {\"$base.$test_func\", $test_func, $setup_func, $teardown_func},"
        else
            echo "    {\"$base.$test_func\", $test_func},"
        fi >> $t2
        ntests=$[ntests+1]
    done
done

(
    echo "#include \"c_unit_fw.h\""
    cat $t1
    echo "const struct c_unit_test _c_unit_tests[] = {"
    cat $t2
    echo "    {0, 0, 0, 0}"
    echo "};"
    echo "const int _c_unit_ntests = $ntests;"
) > $output_file.N && mv -f $output_file.N $output_file
