
# 3 sec
#CUBA_MAXEVAL=5 CUBA_MAXEPS=0.001 CUBA_NSTART=5 CUBA_NINCREASE=50 CUBA_NBATCH=50  ./bin/srm_test -v -t stdp_many

for CUBA_MAXEVAL in {1..20}; do
    (time ./build/bin/srm_test -t stdp_many &> /var/tmp/$CUBA_MAXEVAL.out) 
done

