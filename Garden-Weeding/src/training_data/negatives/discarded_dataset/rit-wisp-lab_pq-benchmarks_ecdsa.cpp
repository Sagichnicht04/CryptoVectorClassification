#include <botan/auto_rng.h>
#include <botan/ec_group.h>
#include <botan/ecdsa.h>
#include <botan/pubkey.h>

#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>

#include "util.h"

int sign_and_verify() {

   Botan::AutoSeeded_RNG rng;
   Botan::ECDSA_PrivateKey key(rng, Botan::EC_Group("secp521r1"));

   const std::string message("This is a tasty burger!");

   Botan::PK_Signer signer(key, rng, "SHA-256");

   auto start = std::chrono::high_resolution_clock::now();
   signer.update(message);
   
   std::vector<uint8_t> signature = signer.signature(rng);
   
   auto stop = std::chrono::high_resolution_clock::now();
   sign_times.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count());

   Botan::PK_Verifier verifier(key, "SHA-256");
   
   start = std::chrono::high_resolution_clock::now();
   verifier.update(message);
   stop = std::chrono::high_resolution_clock::now();
   
   std::cout << "is " << (verifier.check_signature(signature) ? "valid" : "invalid") << std::endl;
   verify_times.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count());
   return 0;
}

int main() {
	unsigned long long average_sign_time, average_verify_time, stddev_sign_time, stddev_verify_time;	
	for(int i = 0; i < ITERATIONS; i++) {
		std::cout << "i=" << i << "\t";
		sign_and_verify();
	}
	get_average_sign_time(average_sign_time, sign_times);
	get_average_verify_time(average_verify_time, verify_times);
	get_stddev_sign(stddev_sign_time, sign_times);
	get_stddev_verify(stddev_verify_time, verify_times);

    record_results(average_sign_time,
                   average_verify_time,
                   stddev_sign_time,
                   stddev_verify_time,
                   "ecdsa_results.txt");

	sign_times.clear();
	verify_times.clear();
	
	return 0;
}
