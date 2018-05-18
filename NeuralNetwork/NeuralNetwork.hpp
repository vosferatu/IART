#ifndef NEURALNETWORK_HPP
#define NEURALNETWORK_HPP

#include <iostream>
#include <array>
#include <vector>
#include <numeric>
#include <algorithm>
#include <execution>
#include <atomic>
#include <random>

namespace ANN
{
	/***************************************************************************************************
	***** Pseudo-Random Number Generator ***************************************************************
	***************************************************************************************************/

	class GaussianRandomGenerator
	{
	public:
		GaussianRandomGenerator() :
			_seeds(std::transform(std::begin(_random_data), std::end(_random_data), std::begin(_random_data),
								  [this] (auto const &) { return _random_device(); }),
				   std::end(_random_data)),
			_engine(_seeds),
			_numbers(0.0, 1.0)
		{
		}

		double operator()()
		{
			return _numbers(_engine);
		}
	private:
		static std::random_device _random_device;
		static std::array<std::mt19937::result_type, std::mt19937::state_size> _random_data;

		std::seed_seq _seeds;
		std::mt19937 _engine;
		std::normal_distribution<> _numbers;
	};

	std::random_device GaussianRandomGenerator::_random_device;
	std::array<std::mt19937::result_type, std::mt19937::state_size> GaussianRandomGenerator::_random_data;

	inline GaussianRandomGenerator GRG;

	/***************************************************************************************************
	***** Initialization functions *********************************************************************
	***************************************************************************************************/

	template <class NeuronType>
	inline std::vector<typename NeuronType::reference_type> _initialize_vector(std::vector<NeuronType> & input)
	{
		std::vector<typename NeuronType::reference_type> vector;
		vector.reserve(std::size(input));

		std::for_each(std::begin(input), std::end(input), [&vector] (auto & neuron) {
			vector.emplace_back(neuron.out());
		});

		return vector;
	}

	template <class NeuronType>
	inline std::vector<std::vector<NeuronType>> _initialize_matrix(std::vector<typename NeuronType::value_type> & input, std::initializer_list<size_t> const & layers)
	{
		std::vector<std::vector<NeuronType>> matrix;
		matrix.reserve(std::size(layers));

		auto begin = std::begin(layers);
		matrix.emplace_back(*begin++, input);
		std::for_each(begin, std::end(layers), [&matrix] (size_t const & layer) {
			matrix.emplace_back(layer, matrix.back());
		});

		return matrix;
	}

	/***************************************************************************************************
	***** Neuron ***************************************************************************************
	***************************************************************************************************/

	class Neuron
	{
	public:
		using value_type = double;
		using reference_type = std::reference_wrapper<double>;
	protected:
		Neuron(std::vector<value_type> & input) :
			_input(std::begin(input), std::end(input)),
			_output(0.0)
		{
		}
		template <class NeuronType>
		Neuron(std::vector<NeuronType> & input) :
			_input(_initialize_vector(input)),
			_output(0.0)
		{
		}
		Neuron(Neuron const &) = default;
	public:
		value_type & out()
		{
			return _output;
		}
	protected:
		std::vector<reference_type> _input;
		value_type _output;
	};

	class SigmoidNeuron : public Neuron
	{
	public:
		SigmoidNeuron(std::vector<value_type> & input) : Neuron(input),
			_weights(std::size(input)),
			_bias(GRG())
		{
			std::generate(std::begin(_weights), std::end(_weights), std::ref(GRG));
		}
		SigmoidNeuron(std::vector<SigmoidNeuron> & input) : Neuron(input),
			_weights(std::size(input)),
			_bias(GRG())
		{
			std::generate(std::begin(_weights), std::end(_weights), std::ref(GRG));
		}
		SigmoidNeuron(SigmoidNeuron const & other) : Neuron(other),
			_weights(std::size(other._weights)),
			_bias(GRG())
		{
			std::generate(std::begin(_weights), std::end(_weights), std::ref(GRG));
		}

		void operator()()
		{
			// _output = 1.0 / (1.0 + std::exp(-std::inner_product(std::begin(_inputs), std::end(_inputs), std::begin(_weights), 0.0) - _bias));
		}
	private:
		std::vector<value_type> _weights;
		value_type _bias;
	};

	/***************************************************************************************************
	***** NeuralNetwork ********************************************************************************
	***************************************************************************************************/

	template <class NeuronType>
	class NeuralNetwork
	{
		static_assert(std::is_base_of_v<Neuron, NeuronType>, "NeuronType must derive from Neuron");
	public:
		NeuralNetwork<NeuronType>(size_t input, std::initializer_list<size_t> layers) :
			_input(input),
			_network(_initialize_matrix<NeuronType>(_input, layers)),
			_output(_initialize_vector(_network.back()))
		{
		}

		// TODO
		// Training function
		// Testing function
	private:
		std::vector<typename NeuronType::value_type> _input;
		std::vector<std::vector<NeuronType>> _network;
		std::vector<typename NeuronType::reference_type> _output;
	};
}

#endif
