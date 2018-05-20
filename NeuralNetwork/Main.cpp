#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#define ANN_DEBUG
#include "NeuralNetwork.hpp"

struct Frog
{
	size_t ID;
	std::string species;
	std::string genus;
	std::string family;
	std::vector<double> MFCCs{ std::vector<double>(22, 0.0) };
	std::vector<double> taxonomy{ std::vector<double>(10, 0.0) };
	bool target{ false };

	std::vector<double> & input()
	{
		return MFCCs;
	}
	std::vector<double> & output()
	{
		return taxonomy;
	}

	bool operator==(Frog const & other)
	{
		return ID == other.ID;
	}
	bool operator!=(Frog const & other)
	{
		return ID != other.ID;
	}
};

std::vector<Frog> read_file()
{
	std::fstream file("Frogs_MFCCs.csv", std::ios::in);
	file.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Ignore first line

	std::vector<std::string> file_string(7195);
	std::vector<Frog> file_data(7195);

	for (std::string & string : file_string)
	{
		std::getline(file, string);
	}

	file.close();

	int species_index = -1;
	std::string species_name = "";

	auto data = std::begin(file_data);

	for (std::string & string : file_string)
	{
		std::stringstream stream(string);

		for (double & MFCC : data->MFCCs)
		{
			stream >> MFCC;
			stream.ignore(std::numeric_limits<std::streamsize>::max(), ',');
		}

		std::getline(stream, data->family, ',');
		std::getline(stream, data->genus, ',');
		std::getline(stream, data->species, ',');

		data->taxonomy[species_name != data->species ? species_name = data->species, ++species_index : species_index] = 1.0;

		stream >> data->ID;
		stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

		++data;
	}

	return file_data;
}

// REQUIRES C++17
int main()
{
	std::ios_base::sync_with_stdio(false);
	std::cout << std::scientific << std::fixed;
	std::cout.precision(9);

	ANN::NeuralNetwork<Frog, ANN::Sigmoid, ANN::MeanSquared> ann({ 22, 16, 10, 4, 8, 10 }, 0.5);

	std::vector<Frog> data = read_file();

//	std::shuffle(std::begin(data), std::end(data), [] { return ANN::RNG.uniform(); }); // Shuffle, pretty self-explanatory
	
	std::vector<Frog> testing_set(std::size(data));
	std::vector<Frog> training_set(std::size(data));
	
	auto range = std::partition_copy(std::begin(data), std::end(data), std::begin(testing_set), std::begin(training_set), [&] (auto & frog) { return data.front() == frog; }); // Leave-one-out CV
	testing_set.erase(range.first, std::end(testing_set));
	training_set.erase(range.second, std::end(training_set));

	std::for_each(std::begin(training_set), std::end(training_set), [&] (auto & frog) { frog.target = testing_set.front().species == frog.species; });

//	std::shuffle(std::begin(testing_set), std::end(testing_set), [] { return ANN::RNG.uniform(); });
//	std::shuffle(std::begin(training_set), std::end(training_set), [] { return ANN::RNG.uniform(); });

	for (;;)
	{
		ann.train(training_set);
	//	ann.test(testing_set);
	}

	// TODO
	// Evaluate results
	// Output results

	return 0;
}
