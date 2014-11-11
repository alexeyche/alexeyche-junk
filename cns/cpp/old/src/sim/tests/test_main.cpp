

#include "rand_tests.cpp"
#include "axon_test.cpp"

int main(int argc, char **argv) {
	CLI::ParseCommandLine(argc, argv);
    Log::Warn << "Tests started" << std::endl;
	Log::Warn << "===== test_sample_rand_gen =====" << std::endl;
    test_sample_rand_gen();
    Log::Warn << "===== end =====" << std::endl;
    Log::Warn << "===== axon_test =====" << std::endl;
    axon_test();
    Log::Warn << "===== end =====" << std::endl;
}    
