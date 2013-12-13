for j in {1..3}; do 
    for i in {1..10}; do 
        ./bin/srm_sim -m run -p /var/tmp/d${j}_${i}.csv --pattdur 750 -t 750 -e 10 -v; 
    done; 
done
