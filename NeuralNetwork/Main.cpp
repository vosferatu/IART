#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "NeuralNetwork.hpp"

struct Frog
{
	size_t ID;
	std::string species;
	std::string genus;
	std::string family;
	std::vector<double> MFCCs{ std::vector<double>(22, 0.0) };
	std::vector<double> taxonomy{ std::vector<double>(10, 0.0) };

	std::vector<double> & input()
	{
		return MFCCs;
	}
	std::vector<double> & output()
	{
		return taxonomy;
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
	ANN::NeuralNetwork<Frog, ANN::Sigmoid, ANN::MeanSquared> ann({ 22, 16, 10, 4, 8, 10 }, 0.01);

	std::vector<Frog> data = read_file();

	std::cout << std::scientific << std::fixed;
	std::cout.precision(6);

	ann.train(data);
	// TODO
	// Evaluate results
	// Output results

	return 0;
}
