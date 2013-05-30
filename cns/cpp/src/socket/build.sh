set -x
rm -rf ./*.o
g++ ./test_sock_main.cpp -lpthread -O2 -o test
g++ ./test_sock_main_serv.cpp -lpthread -O2 -o test_s
R CMD SHLIB ./sim_socket_r.c
