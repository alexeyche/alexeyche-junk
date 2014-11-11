
# 3 sec
#CUBA_MAXEVAL=5 CUBA_MAXEPS=0.001 CUBA_NSTART=5 CUBA_NINCREASE=50 CUBA_NBATCH=50  ./bin/srm_test -v -t stdp_many

time_filename=/var/tmp/time_runs
[ -f $time_filename ] && rm $time_filename
#CUBA_MAXEVAL=5 CUBA_MAXEPS=0.001 CUBA_NSTART=5 CUBA_NINCREASE=50 CUBA_NBATCH=50

export CUBA_MAXEPS=0.001
export CUBA_NSTART=5
export CUBA_NINCREASE=50
export CUBA_NBATCH=50

for CUBA_NBATCH in `seq 50 5 80`; do
    export CUBA_NBATCH=$CUBA_NBATCH
    for CUBA_NSTART in {3..15}; do
        export CUBA_NSTART=$CUBA_NSTART
        for CUBA_MAXEVAL in {5..15}; do
            export CUBA_MAXEVAL=$CUBA_MAXEVAL
            
            filename=/var/tmp/${CUBA_NBATCH}_${CUBA_NSTART}_${CUBA_MAXEVAL}_out.csv
            echo -n "$filename"  >> $time_filename
            echo -n " " >> $time_filename
            ../build/bin/srm_test -t stdp_many -o $filename | tr '\n' ' ' >> $time_filename
            ./compare_best_shot.R $filename >> $time_filename
        done
    done
done    

