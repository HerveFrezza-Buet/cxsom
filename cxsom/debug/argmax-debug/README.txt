make rule-xxx

rm -rf root-dir
mkdir root-dir

make cxsom-set-config ROOT_DIR=./root-dir HOSTNAME=localhost PORT=10000 NB_THREADS=4
make cxsom-launch-processor
make cxsom-scan-vars 

make send-rules
