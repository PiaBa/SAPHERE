#include "seal/seal.h"
#include <chrono>

using std::cout, std::endl;

/*
* Converts a time double of nanoseconds into a time double of milliseconds
*/
double nano_to_millisecond(double number) {
	return number / 1000000;
}

/*
* Converts a fraction into a percentage
*/
double to_percentage(double number) {
	return number * 100;
}

/*
* Calculates the mean of a vector containing double elements
*/
double calc_mean(std::vector<double> vec) {
	auto size = vec.size();

	if (size == 0) {
		throw std::logic_error("Cannot calculate the mean of an empty vector.");
	}

	double sum(0);
	for (size_t i = 0; i < size; i++) {
		sum += vec[i];
	}

	return static_cast<double>(sum) / static_cast<double>(size);
}

/*
* Calculates the minimum of a vector containing double elements
*/
double calc_minimum(std::vector<double> vec) {
	if (vec.size() == 0) {
		throw std::logic_error("Cannot calculate the minimum of an empty vector.");
	}
	
	return *min_element(vec.begin(), vec.end());
}

/*
* Calculates the maximum of a vector containing double elements
*/
double calc_maximum(std::vector<double> vec) {
	if (vec.size() == 0) {
		throw std::logic_error("Cannot calculate the maximum of an empty vector.");
	}

	return *max_element(vec.begin(), vec.end());
}

/*
* Calculates the median of a vector containing double elements
*/
double calc_median(std::vector<double> vec) {
	auto size = vec.size();
	
	if (size == 0) {
		throw std::logic_error("Cannot calculate the median of an empty vector.");
	}

	if (size % 2) { //Odd vector size
		auto index = vec.size() / 2;
		nth_element(vec.begin(), vec.begin() + index, vec.end());
		return vec[index];
	}
	else { //Even vector size
		auto index_first = vec.size() / 2 - 1;
		auto index_second = vec.size() / 2;

		nth_element(vec.begin(), vec.begin() + index_first, vec.end());
		double value_first = vec[index_first];

		nth_element(vec.begin(), vec.begin() + index_second, vec.end());
		double value_second = vec[index_second];

		return (value_first + value_second) / 2.0;
	}	
}

/*
* Calculates the sample variance of a vector containing double elements
*/
double calc_variance(std::vector<double> vec) {
	auto size = vec.size();
	auto mean = calc_mean(vec);
	auto squared_diffs = 0.0;
	 
	for (size_t i = 0; i < size; i++) {
		auto diff = vec[i] - mean;
		squared_diffs += diff * diff;
	}
	return squared_diffs / (size - 1);
}

/*
* Calculates the sample standard deviation of a vector containing double elements
*/
double calc_std_deviation(std::vector<double> vec) {
	return sqrt(calc_variance(vec));
}

/*
* Prints the first cnt elements of a vector of doubles
*/
void print_vec(std::vector<double> vec, size_t cnt) {
	cout << std::fixed << std::setprecision(15);
	if (vec.size() >= cnt) {
		cout << "[";
		for (size_t i = 0; i < cnt; i++) {
			cout << vec[i] << ", ";
		}
		cout << "...]" << endl << endl;
	}
	else {
		throw std::invalid_argument("Received a cnt value less than the vector size.");
	}
}

/*
* Prints a description and a time value of nanoseconds in milliseconds
*/
void print_time(std::string description, double value) {
	cout << std::fixed << std::setprecision(6);
	cout << std::setw(100) << std::left << description << std::setw(30) << std::right << nano_to_millisecond(value) << " ms" << endl;
}

/*
* Prints a description and an error value
*/
void print_error(std::string description, double value) {
	cout << std::fixed << std::setprecision(15);
	cout << std::setw(105) << std::left << description << std::setw(30) << std::right << value << endl;
}

/*
* Prints a description and a percentage
*/
void print_percentage(std::string description, double value) {
	cout << std::fixed << std::setprecision(4);
	cout << std::setw(100) << std::left << description << std::setw(30) << std::right << value << " %" << endl;
}


int main() {
	cout << "--- Preparations ---" << endl;

	//Create params for the data owner
	cout << "Creating params for the data owner..." << endl;;
	seal::EncryptionParameters params_d(seal::scheme_type::ckks);
	size_t poly_modulus_degree_d = 8192;
	params_d.set_poly_modulus_degree(poly_modulus_degree_d);
	params_d.set_coeff_modulus(seal::CoeffModulus::Create(poly_modulus_degree_d, { 60, 40, 40, 60 }));
	double scale_d = pow(2.0, 40);
	seal::SEALContext context_d(params_d);

	//Create params for processor
	cout << "Creating params for the processor..." << endl << endl;;
	seal::EncryptionParameters params_p(seal::scheme_type::ckks);
	size_t poly_modulus_degree_p = 8192;
	params_p.set_poly_modulus_degree(poly_modulus_degree_p);
	params_p.set_coeff_modulus(seal::CoeffModulus::Create(poly_modulus_degree_p, { 60, 40, 40, 60 }));
	double scale_p = pow(2.0, 40);
	seal::SEALContext context_p(params_p);

	//Create keys + encoder for the data owner
	cout << "Creating keys and encoder for the data owner..." << endl;
	seal::KeyGenerator keygen_d(context_d);
	auto secret_key_d = keygen_d.secret_key();
	seal::PublicKey public_key_d;
	keygen_d.create_public_key(public_key_d);
	seal::Encryptor encryptor_d(context_d, public_key_d);
	seal::Decryptor decryptor_d(context_d, secret_key_d);
	seal::CKKSEncoder encoder_d(context_d);

	//Create keys + encoder for the processor
	cout << "Creating keys and encoder for the processor..." << endl << endl;
	seal::KeyGenerator keygen_p(context_p);
	auto secret_key_p = keygen_p.secret_key();
	seal::PublicKey public_key_p;
	keygen_p.create_public_key(public_key_p);
	seal::Encryptor encryptor_p(context_p, public_key_p);
	seal::Decryptor decryptor_p(context_p, secret_key_p);
	seal::CKKSEncoder encoder_p(context_p);

	//Create the initial vector
	double initial_number = 9;

	cout << "Creating the initial vector..." << endl;
	std::vector<double> vec;
	size_t slot_count = encoder_d.slot_count();
	vec.reserve(slot_count);
	for (size_t i = 0; i < slot_count; i++) {
		vec.push_back(initial_number); //Filling all vector slots with an exemplary number
	}
	cout << "The initial vector is: " << endl;
	print_vec(vec, 5);

	//Data owner encodes and encrypts the vector
	cout << "Data owner encodes and encrypts the vector..." << endl;
	seal::Plaintext vec_encoded;
	encoder_d.encode(vec, scale_d, vec_encoded);

	seal::Ciphertext vec_encrypted;
	encryptor_d.encrypt(vec_encoded, vec_encrypted);

	//Data owner sends the resulting ciphertext to the processor, which performs operations on the data
	cout << "In theory: Data owner sends the resulting ciphertext to the processor, which performs operations on the data..." << endl;


	int execution_cnt = 1000;


	//--- Standard version ---
	cout << endl << endl << "--- Standard version ---" << endl;

	std::vector<double> std_times, std_errors;
	std_times.reserve(execution_cnt);
	std_errors.reserve(execution_cnt * slot_count);

	for (size_t i = 0; i < execution_cnt; i++) {
		//Step 1: In theory: Transmission of result back to the data owner
		if (i == 0) {
			cout << "Step 1: In theory: Transmission of the encrypted computation result to the data owner..." << endl;
			cout << "--> Size of the data: " << vec_encrypted.size() * params_p.coeff_modulus().size()
				* params_p.poly_modulus_degree() * sizeof(std::uint64_t) << " Bytes" << endl;
		}	
				
		//Step 2: Data owner decrypts and decodes the vector
		if (i == 0) cout << "Step 2: Data owner decrypts and decodes the vector..." << endl;

		auto std_start = std::chrono::high_resolution_clock::now(); //Start timer

			seal::Plaintext vec_after_decrypt;
			decryptor_d.decrypt(vec_encrypted, vec_after_decrypt); //Data owner decryptsmaximum

			std::vector<double> vec_after_decode;
			encoder_d.decode(vec_after_decrypt, vec_after_decode); //Data owner decodes
		
		auto std_end = std::chrono::high_resolution_clock::now(); //Stop timer

		//Step 3: In theory: Transmission of the result back to the processor
		if (i == 0) {
			cout << "Step 3: In theory: Transmission of the plaintext result back to the processor..." << endl;
			cout << "--> Size of the data: " << vec_after_decode.size() * sizeof(double) << " Bytes" << endl;
			cout << endl << "Exemplary resulting vector of the standard version: " << endl;
			print_vec(vec_after_decode, 5); //Print resulting vector
		}

		std_times.push_back(static_cast<double>((std_end - std_start).count())); //Add time to vector

		for (size_t i = 0; i < slot_count; i++) {
			std_errors.push_back(abs(initial_number - vec_after_decode[i])); //Calculate errors and add to vector
		}
	}

	cout << endl;
	print_time("Minimum execution time of step 2:", calc_minimum(std_times));
	print_time("Maximum execution time of step 2:", calc_maximum(std_times));
	print_time("Median of the execution times of step 2:", calc_median(std_times));
	print_time("Mean of the execution times of step 2:", calc_mean(std_times));
	print_time("Standard deviation of the execution times of step 2:", calc_std_deviation(std_times));

	cout << endl;
	print_error("Minimum error introduced to the vector:", calc_minimum(std_errors));
	print_error("Maximum error introduced to the vector:", calc_maximum(std_errors));
	print_error("Median of the errors introduced to the vector:", calc_median(std_errors));
	print_error("Mean of the errors introduced to the vector:", calc_mean(std_errors));
	print_error("Standard deviation of the errors introduced to the vector:", calc_std_deviation(std_errors));


	//--- SAPHERE ---
	cout << endl << endl << "--- SAPHERE ---" << endl;

	std::vector<double> saph_step1_times, saph_step3_times, saph_step5_times;
	saph_step1_times.reserve(execution_cnt);
	saph_step3_times.reserve(execution_cnt);
	saph_step5_times.reserve(execution_cnt);

	std::vector<double> saph_errors;
	saph_errors.reserve(execution_cnt * slot_count);

	for (size_t i = 0; i < execution_cnt; i++) {
		//Step 1: Processor adds additional encryption to c 
		if (i == 0) cout << "Step 1: Processor adds additional encryption to c" << endl;

		auto saph_step1_start = std::chrono::high_resolution_clock::now(); //Start timer

			//(1a) Processor splits c and encrypts the first part
			uint64_t cipher_size = vec_encrypted.dyn_array().size();
			uint64_t plain_size = cipher_size / 2;

				//Processor turns value of c_first into plaintext
				seal::Plaintext c_first_plain;
				encoder_d.encode(3, scale_p, c_first_plain); //dummy value

				//Processor fills c_first_plain with the content of the first element of c
				for (std::size_t i = 0; i < plain_size; i++) {
					c_first_plain.data()[i] = vec_encrypted.data()[i];
				}

				//Processor adds encryption to c_0
				seal::Ciphertext c_first_ciph;
				encryptor_p.encrypt(c_first_plain, c_first_ciph);


			//(1b) In Theory: Processor saves c'_1 for later decryption

			//(1c) Processor concatinates c'_0 and c_1 to obtain c''
			seal::Ciphertext ciph_dd(c_first_ciph); //Create dummy based on arbitrary ciphertext

				//Fill first element
				for (size_t i = 0; i < cipher_size / 2; i++) {
					ciph_dd.data()[i] = c_first_ciph.data()[i];
				}

				//Fill second element
				for (size_t i = 0; i < cipher_size / 2; i++) {
					ciph_dd.data()[i + (cipher_size / 2)] = vec_encrypted.data()[i + (cipher_size / 2)];
				}

		auto saph_step1_end = std::chrono::high_resolution_clock::now(); //Stop timer
		saph_step1_times.push_back(static_cast<double>((saph_step1_end - saph_step1_start).count())); //Add time to vector

		//Step 2: In theory: Transmission of c'' to the data owner...
		if (i == 0) {
			cout << "Step 2: In theory: Transmission of c'' to the data owner..." << endl;
			cout << "--> Size of the data: " << ciph_dd.size() * params_p.coeff_modulus().size()
				* params_p.poly_modulus_degree() * sizeof(std::uint64_t) << " Bytes" << endl;
		}

		//Step 3: Data owner decrypts c'' to obtain the midtext
		if (i == 0) cout << "Step 3: Data owner decrypts c'' to obtain the midtext..." << endl;

		auto saph_step3_start = std::chrono::high_resolution_clock::now(); //Start timer

			seal::Plaintext midtext;
			decryptor_d.decrypt(ciph_dd, midtext);

		auto saph_step3_end = std::chrono::high_resolution_clock::now(); //Stop timer
		saph_step3_times.push_back(static_cast<double>((saph_step3_end - saph_step3_start).count())); //Add time to vector

		//Step 4: Transmission of the midtext to the processor
		if (i == 0) {
			cout << "Step 4: In theory: Transmission of midtext to processor..." << endl;
			cout << "--> Size of the data: " << params_p.coeff_modulus().size()
				* params_p.poly_modulus_degree() * sizeof(std::uint64_t) << " Bytes" << endl;
		}

		//Step 5: Processor creates c''' by concatenating the midtext and c'_1. It then decrypts and decodes c'''
		if (i == 0) cout << "Step 5: Processor creates c''' by concatenating the midtext and c'_1. It then decrypts c'''..." << endl;

		auto saph_step5_start = std::chrono::high_resolution_clock::now(); //Start timer

			//(5a) Processor concatinates midtext and c'_1 to obtain c'''
			seal::Ciphertext ciph_ddd(c_first_ciph); //Create dummy based on arbitrary ciphertext
				
				//Fill first element
				for (size_t i = 0; i < cipher_size / 2; i++) {
					ciph_ddd.data()[i] = midtext.data()[i];
				}

				//Fill second element
				for (size_t i = 0; i < cipher_size / 2; i++) { 
					ciph_ddd.data()[i + (cipher_size / 2)] = c_first_ciph.data()[i + (cipher_size / 2)];
				}
	
			//(5b) Processor decrypts and decodes c''' to obtain result
			seal::Plaintext result_plain;
			decryptor_p.decrypt(ciph_ddd, result_plain);

			std::vector<double> result_decoded;
			encoder_p.decode(result_plain, result_decoded);

		auto saph_step5_end = std::chrono::high_resolution_clock::now(); //Stop timer
		saph_step5_times.push_back(static_cast<double>((saph_step5_end - saph_step5_start).count())); //Add time to vector

		for (size_t i = 0; i < slot_count; i++) {
			saph_errors.push_back(abs(initial_number - result_decoded[i])); //Calculate errors and add to vector
		}

		if (i == 0) {
			cout << endl << "Exemplary resulting vector of the SAPHERE approach: " << endl;
			print_vec(result_decoded, 5); //Print resulting vector
		}
	}

	cout << endl;

	//Print execution time info (step 1)
	print_time("Minimum execution time of step 1:", calc_minimum(saph_step1_times));
	print_time("Maximum execution time of step 1:", calc_maximum(saph_step1_times));
	print_time("Median of the execution times of step 1:", calc_median(saph_step1_times));
	print_time("Mean of the execution times of step 1:", calc_mean(saph_step1_times));
	print_time("Standard deviation of the execution times of step 1:", calc_std_deviation(saph_step1_times));
	cout << endl;

	//Print execution time info (step 3)
	print_time("Minimum execution time of step 3:", calc_minimum(saph_step3_times));
	print_time("Maximum execution time of step 3:", calc_maximum(saph_step3_times));
	print_time("Median of the execution times of step 3:", calc_median(saph_step3_times));
	print_time("Mean of the execution times of step 3:", calc_mean(saph_step3_times));
	print_time("Standard deviation of the execution times of step 3:", calc_std_deviation(saph_step3_times));
	cout << endl;

	//Print execution time info (step 5)
	print_time("Minimum execution time of step 5:", calc_minimum(saph_step5_times));
	print_time("Maximum execution time of step 5:", calc_maximum(saph_step5_times));
	print_time("Median of the execution times of step 5:", calc_median(saph_step5_times));
	print_time("Mean of the execution times of step 5:", calc_mean(saph_step5_times));
	print_time("Standard deviation of the execution times of step 5:", calc_std_deviation(saph_step5_times));
	cout << endl;


	//Calculate aggregated results
	std::vector<double> saph_processor_times, saph_total_times;
	saph_processor_times.reserve(execution_cnt);
	saph_total_times.reserve(execution_cnt);

	for (size_t i = 0; i < execution_cnt; i++) {
		saph_processor_times.push_back(saph_step1_times[i] + saph_step5_times[i]);
		saph_total_times.push_back(saph_step1_times[i] + saph_step3_times[i] + saph_step5_times[i]);
	}

	//Print aggregated execution times info for the processor (step 1 and 5)
	print_time("Minimum execution time at the processor (step 1 and 5):", calc_minimum(saph_processor_times));
	print_time("Maximum execution time at the processor (step 1 and 5):", calc_maximum(saph_processor_times));
	print_time("Median of the execution times at the processor (step 1 and 5):", calc_median(saph_processor_times));
	print_time("Mean of the execution times at the processor (step 1 and 5):", calc_mean(saph_processor_times));
	print_time("Standard deviation of the execution times at the processor (step 1 and 5):", calc_std_deviation(saph_processor_times));
	cout << endl;

	//Print aggregated execution times info for the data owner (step 3) (same as before)
	print_time("Minimum execution time at the data owner (step 3):", calc_minimum(saph_step3_times));
	print_time("Maximum execution time at the data owner (step 3):", calc_maximum(saph_step3_times));
	print_time("Median of the execution times at the data owner (step 3):", calc_median(saph_step3_times));
	print_time("Mean of the execution times at the data owner (step 3):", calc_mean(saph_step3_times));
	print_time("Standard deviation of the execution times at the data owner (step 3):", calc_std_deviation(saph_step3_times));
	cout << endl;

	//Print aggregated total execution times (for all the steps)
	print_time("Minimum total execution time:", calc_minimum(saph_total_times));
	print_time("Maximum total execution time:", calc_maximum(saph_total_times));
	print_time("Median of the total execution times:", calc_median(saph_total_times));
	print_time("Mean of the total execution times:", calc_mean(saph_total_times));
	print_time("Standard deviation of the total execution times:", calc_std_deviation(saph_total_times));
	cout << endl;

	//Print error information
	print_error("Minimum error introduced to the vector:", calc_minimum(saph_errors));
	print_error("Maximum error introduced to the vector:", calc_maximum(saph_errors));
	print_error("Median of the errors introduced to the vector:", calc_median(saph_errors));
	print_error("Mean of the errors introduced to the vector:", calc_mean(saph_errors));
	print_error("Standard deviation of the errors introduced to the vector:", calc_std_deviation(saph_errors));

	//Print relative values
	cout << endl << endl << "--- Relative evaluation ---" << endl;	
	print_percentage("Relative change in the execution time at the data owner: (considering the median)", to_percentage(calc_median(saph_step3_times) / calc_median(std_times)));
	print_percentage("Relative change in the execution time at the data owner: (considering the mean)", to_percentage(calc_mean(saph_step3_times) / calc_mean(std_times)));
	cout << endl;

	print_percentage("Relative change in the total execution time: (considering the median)", to_percentage(calc_median(saph_total_times) / calc_median(std_times)));
	print_percentage("Relative change in the total execution time: (considering the mean)", to_percentage(calc_mean(saph_total_times) / calc_mean(std_times)));
	cout << endl;

	print_percentage("Relative change in the introduced error: (considering the median)", to_percentage(calc_median(saph_errors) / calc_median(std_errors)));
	print_percentage("Relative change in the introduced error: (considering the mean)", to_percentage(calc_mean(saph_errors) / calc_mean(std_errors)));
	cout << endl;
}