#ifndef NEURALNETWORK_HPP
#define NEURALNETWORK_HPP

#include <array>
#include <vector>
#include <random>
#include <numeric>
#include <algorithm>
#include <execution>
#include <atomic>

namespace ANN
{
#define ANN_TMP_OPT

	using double_ref = std::reference_wrapper<double>;

	inline constexpr double INF = std::numeric_limits<double>::infinity();

	/***************************************************************************************************
	***** Pseudo-Random Number Generator ***************************************************************
	***************************************************************************************************/

	class RandomGenerator
	{
	public:
		RandomGenerator() :
			_seeds(std::begin(_random_data),
				   std::transform(std::begin(_random_data), std::end(_random_data), std::begin(_random_data),
								  [this] (auto const &) { return _random_device(); })),
			_engine(_seeds),
			_gaussian(0.0, 0.1)
		{
		}

		int uniform()
		{
			return _uniform(_engine);
		}
		double gaussian()
		{
			return _gaussian(_engine);
		}
	private:
		std::random_device _random_device{};
		std::array<std::mt19937_64::result_type, std::mt19937_64::state_size> _random_data{};
		std::seed_seq _seeds;
		std::mt19937_64 _engine;
		std::uniform_int_distribution<int> _uniform;
		std::normal_distribution<double> _gaussian;
	};

	inline RandomGenerator RNG;

	/***************************************************************************************************
	***** Functions ************************************************************************************
	***************************************************************************************************/

	// Activation functions

	struct Sigmoid
	{
		static double function(double x)
		{
			return 1.0 / (1.0 + std::exp(-x));
		}
		static double derivative(double x)
		{
			return std::exp(x) / std::pow(std::exp(x) + 1, 2);
		}
	};

	struct Softmax
	{
		static double function(double x, std::vector<double> xl)
		{
			return std::exp(x) / std::transform_reduce(std::execution::seq, std::begin(xl), std::end(xl), 0.0, std::plus<>(), [] (auto & x) { return std::exp(x); });
		}
		static double derivative(double x)
		{
			// Check this
			return x;
		}
	};
	
	// Cost functions

	struct MeanSquared
	{
		static double function(double a, double y)
		{
			return std::pow(a - y, 2) / 2;
		}
		static double derivative(double a, double y)
		{
			return a - y;
		}
		static std::vector<double> weight_derivative(std::vector<double> al, double e)
		{
			std::for_each(std::execution::seq, std::begin(al), std::end(al), [&] (auto & ak) {
				ak *= e;
			});
			return al;
		}
		static double bias_derivative(double e)
		{
			return e;
		}
	};

	struct LogLikelihood
	{
		static double derivative(double a, double y)
		{
			// Check this
			return a - y;
		}
		static std::vector<double> weight_derivative(std::vector<double> al, double a, double y)
		{
			double c = a - y;
			std::for_each(std::execution::seq, std::begin(al), std::end(al), [&] (auto & ak) {
				ak *= c;
			});
			return al;
		}
		static double bias_derivative(double a, double y)
		{
			return a - y;
		}
	};

	/***************************************************************************************************
	***** Neuron ***************************************************************************************
	***************************************************************************************************/

	class Neuron
	{
	public:
		Neuron(std::vector<double> & input) : 
			_input(std::begin(input), std::end(input)),
			_weights(std::size(_input))
		{
			std::generate(std::begin(_weights), std::end(_weights), [] { return RNG.gaussian(); });
		}
		Neuron(std::vector<Neuron> & input) :
			_input(neuron_vector(input)),
			_weights(std::size(_input))
		{
			std::generate(std::begin(_weights), std::end(_weights), [] { return RNG.gaussian(); });
		}
		Neuron(Neuron const & other) :
			_input(other._input),
			_weights(std::size(_input))
		{
			std::generate(std::begin(_weights), std::end(_weights), [] { return RNG.gaussian(); });
		}
	
		double feedforward(Sigmoid)
		{
			return _output = Sigmoid::function(_weighted_input);
		}
		double feedforward(Softmax, std::vector<double> weighted_inputs)
		{
			return _output = Softmax::function(_weighted_input, weighted_inputs);
		}
		double backpropagate(Sigmoid, double gradient)
		{
			return _error = gradient * Sigmoid::derivative(_weighted_input);
		}
		double backpropagate(Softmax, double gradient)
		{
			return _error = gradient;
		}

		// Setters
		void calculate()
		{
			_weighted_input = std::inner_product(std::begin(_input), std::end(_input), std::begin(_weights), 0.0) + _bias;
		}
		void clear()
		{
			_weighted_input = INF;
			_output = INF;
			_error = INF;
		}

		// Getters
		std::vector<double> weights()
		{
			return _weights;
		}
		double bias()
		{
			return _bias;
		}
		double weighted_input()
		{
			return _weighted_input;
		}

		void learn(std::vector<double> weights, double bias, double rate)
		{
			std::transform(std::begin(_weights), std::end(_weights), std::begin(weights), std::begin(_weights), [&] (double & weight, double & delta) {
				return weight + rate * delta;
			});
			_bias += rate * bias;
		}
	private:
		std::vector<double_ref> _input;
		std::vector<double> _weights;
		double _bias{ 0.0 };
		double _weighted_input{ INF };
		double _output{ INF };
		double _error{ INF };

		friend std::vector<double_ref> neuron_vector(std::vector<Neuron> &);
	};

	inline std::vector<double_ref> neuron_vector(std::vector<Neuron> & neurons)
	{
		std::vector<double_ref> vector;
		vector.reserve(std::size(neurons));

		std::for_each(std::begin(neurons), std::end(neurons), [&vector] (Neuron & neuron) {
			vector.emplace_back(neuron._output);
		});

		return vector;
	}

	/***************************************************************************************************
	***** NeuralNetwork ********************************************************************************
	***************************************************************************************************/

	template <class DataType, class ActivationFunction, class CostFunction>
	class NeuralNetwork
	{
		template <class Type>
		using matrix = std::vector<std::vector<Type>>;
	public:
		NeuralNetwork(std::initializer_list<size_t> layers, double eta) :
			_eta(eta),
			_input(*std::begin(layers)),
			_network(network_matrix(layers)),
			_output(neuron_vector(_network.back()))
		{
		}
		
		void train(std::vector<DataType> & training_set)
		{
			// Create weight deltas vectors
			matrix<std::vector<double>> delta_weights(std::size(_network)); // Matrix of vectors of delta weights (vector per neuron, vector per layer, vector per network)

			// Initialize weight deltas vectors
			std::transform(std::execution::seq, std::begin(_network), std::end(_network), std::begin(delta_weights), [] (auto & layer) {
				matrix<double> delta(std::size(layer));
				std::transform(std::execution::seq, std::begin(layer), std::end(layer), std::begin(delta), [] (auto & neuron) {
					return std::vector<double>(std::size(neuron.weights()), 0.0);
				});
				return delta;
			});

			// Create biases deltas
			matrix<double> delta_biases(std::size(_network)); // Vector of vectors of delta biases

			// Initialize biases deltas
			std::transform(std::execution::seq, std::begin(_network), std::end(_network), std::begin(delta_biases), [] (auto & layer) {
				return std::vector<double>(std::size(layer), 0.0);
			});

			std::for_each(std::execution::seq, std::begin(training_set), std::end(training_set), [&] (auto & example) {
				// Initialize input vector
				std::transform(std::execution::seq, std::begin(example.input()), std::end(example.input()), std::begin(_input), [&] (auto input) {
					return input;
				});

				// Create activation matrix
				matrix<double> activation(std::size(_network) + 1);
				auto act = std::begin(activation);

				// Initialize activation matrix
				*act++ = _input;
				std::transform(std::execution::seq, std::begin(_network), std::end(_network), act, [] (auto & neurons) {
					return std::vector<double>(std::size(neurons));
				});

				// Create error matrix
				matrix<double> error(std::size(_network));
				auto err = std::rbegin(error);

				// Initialize error matrix
				std::transform(std::execution::seq, std::rbegin(_network), std::rend(_network), err, [] (auto & neurons) {
					return std::vector<double>(std::size(neurons));
				});

				// Create output iterator;
				auto out = std::begin(example.output());

				// Feedforward
				_feedforward(ActivationFunction(), act);

				--act;

#ifdef ANN_DEBUG
				std::vector<double> expected, actual;

				std::cout << "Expected: ";
				for (auto & output : example.output())
				{
					std::cout << output << " ";
					expected.emplace_back(output);
				}

				std::cout << "\nActual: ";
				for (auto & output : activation.back())
				{
					std::cout << output << " ";
					actual.emplace_back(output);
				}

				std::cout << "\nCost: " << std::transform_reduce(std::execution::seq, std::begin(expected), std::end(expected), std::begin(actual), 0.0, std::plus<>(), [] (auto & expected, auto & actual) {
					return CostFunction::function(actual, expected);
				}) << "\n";
#endif

				// Backpropagate
				_backpropagate(ActivationFunction(), CostFunction(), act, err, out);

				--err;

				// Update
				_update(CostFunction(), act, err, out, delta_weights, delta_biases);
			});

			// Teach
			auto weights_layer = std::begin(delta_weights);
			auto biases_layer = std::begin(delta_biases);
			std::for_each(std::begin(_network), std::end(_network), [&] (auto & layer) {
				auto weights_neuron = std::begin(*weights_layer);
				auto biases_neuron = std::begin(*biases_layer);
				std::for_each(std::begin(layer), std::end(layer), [&] (auto & neuron) {
					neuron.learn(*weights_neuron++, *biases_neuron++, -_eta / std::size(training_set));
				});
				++weights_layer;
				++biases_layer;
			});
		}
		void test(std::vector<DataType> & testing_set)
		{
			std::for_each(std::execution::seq, std::begin(testing_set), std::end(testing_set), [&] (auto & example) {
				// Initialize input vector
				std::transform(std::execution::seq, std::begin(example.input()), std::end(example.input()), std::begin(_input), [&] (auto input) {
					return input;
				});

				// Create activation matrix
				matrix<double> activation(std::size(_network) + 1);
				auto act = std::begin(activation);

				// Initialize activation matrix
				*act++ = _input;
				std::transform(std::execution::seq, std::begin(_network), std::end(_network), act, [] (auto & neurons) {
					return std::vector<double>(std::size(neurons));
				});

				// Feedforward
				_feedforward(ActivationFunction(), act);

#ifdef ANN_DEBUG
				std::vector<double> expected, actual;

				std::cout << "Expected: ";
				for (auto & output : example.output())
				{
					std::cout << output << " ";
					expected.emplace_back(output);
				}

				std::cout << "\nActual: ";
				for (auto & output : activation.back())
				{
					std::cout << output << " ";
					actual.emplace_back(output);
				}

				std::cout << "\nCost: " << std::transform_reduce(std::execution::seq, std::begin(expected), std::end(expected), std::begin(actual), 0.0, std::plus<>(), [] (auto & expected, auto & actual) {
					return CostFunction::function(actual, expected);
				}) << "\n";
#endif
			});
		}
	private:
		template <class Iterator>
		void _feedforward(Sigmoid, Iterator & act)
		{
#ifdef ANN_TMP_OPT
			std::for_each(std::execution::seq, std::begin(_network), std::prev(std::end(_network)), [&] (auto & layer) {
#else
			std::for_each(std::execution::seq, std::begin(_network), std::end(_network), [&] (auto & layer) {
#endif
				std::transform(std::execution::seq, std::begin(layer), std::end(layer), std::begin(*act++), [&] (auto & neuron) { // par
					// Calculate weighted_input
					neuron.calculate();

					// Place result in activation
					return neuron.feedforward(Sigmoid());
				});
			});
#ifdef ANN_TMP_OPT
			_feedforward(Softmax(), act);
#endif
		}
		template <class Iterator>
		void _feedforward(Softmax, Iterator & act)
		{
#ifdef ANN_TMP_OPT
			std::for_each(std::execution::seq, std::prev(std::end(_network)), std::end(_network), [&] (auto & layer) {
#else
			std::for_each(std::execution::seq, std::begin(_network), std::end(_network), [&] (auto & layer) {
#endif
				std::vector<double> weighted_inputs(std::size(layer));

				std::transform(std::execution::seq, std::begin(layer), std::end(layer), std::begin(weighted_inputs), [&] (auto & neuron) {
					// Calculate weighted_input
					neuron.calculate();

					// Populate weighted_inputs vector
					return neuron.weighted_input();
				});

				std::transform(std::execution::seq, std::begin(layer), std::end(layer), std::begin(*act++), [&] (auto & neuron) { // par
					// Place result in activation
					return neuron.feedforward(Softmax(), weighted_inputs);
				});
			});
		}

		template <class Iterator1, class Iterator2, class Iterator3>
		void _backpropagate(Sigmoid, MeanSquared, Iterator1 & act, Iterator2 & err, Iterator3 & out)
		{
#ifdef ANN_TMP_OPT
			std::vector<double> gradient = _backpropagate(Softmax(), LogLikelihood(), act, err, out);

			std::for_each(std::execution::seq, std::next(std::rbegin(_network)), std::rend(_network), [&] (auto & layer) {
#else
			// Create gradient vector and calculate gradient for output layer
			std::vector<double> gradient(std::size(_network.back()));
			std::transform(std::execution::seq, std::begin(*act), std::end(*act), out, std::begin(gradient), [&] (auto & actual, auto & expected) {
				return MeanSquared::derivative(actual, expected); // Probably need to do some preprocessing of the expected value...
			});

			std::for_each(std::execution::seq, std::rbegin(_network), std::rend(_network), [&] (auto & layer) {
#endif
				--act;

				// Create transposed weight matrix
				matrix<double> transpose_weights(std::size(*act), std::vector<double>(std::size(layer)));
				std::vector<size_t> index(std::size(layer)); // We do this to be able to easily paralelize the error calculation
				std::iota(std::begin(index), std::end(index), 0);

				// Calculate error
				std::transform(std::execution::seq, std::begin(layer), std::end(layer), std::begin(index), std::begin(*err), [&] (auto & neuron, auto & idx1) { // par
					size_t idx2 = 0;
					auto weights = neuron.weights();

					// Populate transposed weight matrix 
					std::for_each(std::execution::seq, std::begin(weights), std::end(weights), [&] (auto & value) { // par
						transpose_weights.at(idx2++).at(idx1) = value;
					});

					return neuron.backpropagate(Sigmoid(), gradient.at(idx1));
				});

				// Calculate gradient for previous layer
				gradient.resize(std::size(transpose_weights));
				std::transform(std::execution::seq, std::begin(transpose_weights), std::end(transpose_weights), std::begin(gradient), [&] (auto & weight) { // par
					return std::inner_product(std::begin(weight), std::end(weight), std::begin(*err), 0.0);
				});

				++err;
			});
		}
		template <class Iterator1, class Iterator2, class Iterator3>
#ifdef ANN_TMP_OPT
		std::vector<double> _backpropagate(Softmax, LogLikelihood, Iterator1 & act, Iterator2 & err, Iterator3 & out)
#else
		void _backpropagate(Softmax, LogLikelihood, Iterator1 & act, Iterator2 & err, Iterator3 & out)
#endif
		{
			// Create gradient vector and calculate gradient for output layer
			std::vector<double> gradient(std::size(_network.back()));
			std::transform(std::execution::seq, std::begin(*act), std::end(*act), out, std::begin(gradient), [&] (auto & actual, auto & expected) {
				return LogLikelihood::derivative(actual, expected);
			});

#ifdef ANN_TMP_OPT
			std::for_each(std::execution::seq, std::rbegin(_network), std::next(std::rbegin(_network)), [&] (auto & layer) {
#else
			std::for_each(std::execution::seq, std::rbegin(_network), std::rend(_network), [&] (auto & layer) {
#endif
				--act;

				// Create transposed weight matrix
				matrix<double> transpose_weights(std::size(*act), std::vector<double>(std::size(layer)));
				std::vector<size_t> index(std::size(layer)); // We do this to be able to easily paralelize the error calculation
				std::iota(std::begin(index), std::end(index), 0);

				// Calculate error
				std::transform(std::execution::seq, std::begin(layer), std::end(layer), std::begin(index), std::begin(*err), [&] (auto & neuron, auto & idx1) { // par
					size_t idx2 = 0;
					auto weights = neuron.weights();

					// Populate transposed weight matrix 
					std::for_each(std::execution::seq, std::begin(weights), std::end(weights), [&] (auto & value) { // par
						transpose_weights.at(idx2++).at(idx1) = value;
					});

					return neuron.backpropagate(Softmax(), gradient.at(idx1));
				});

				// Calculate gradient for previous layer
				gradient.resize(std::size(transpose_weights));
				std::transform(std::execution::seq, std::begin(transpose_weights), std::end(transpose_weights), std::begin(gradient), [&] (auto & weight) { // par
					return std::inner_product(std::begin(weight), std::end(weight), std::begin(*err), 0.0);
				});

				++err;
			});
#ifdef ANN_TMP_OPT
			return gradient;
#endif
		}

		template <class Iterator1, class Iterator2, class Iterator3>
		void _update(MeanSquared, Iterator1 & act, Iterator2 & err, [[maybe_unused]] Iterator3 & out, matrix<std::vector<double>> & weights, matrix<double> & biases)
		{
#ifdef ANN_TMP_OPT
			std::transform(std::execution::seq, std::begin(weights), std::prev(std::end(weights)), act, std::begin(weights), [&] (auto & layer, auto & activation) {
				++act;
#else
			// Calculate delta weight for this layer
			std::transform(std::execution::seq, std::begin(weights), std::end(weights), act, std::begin(weights), [&] (auto & layer, auto & activation) {
#endif
				matrix<double> delta(std::size(layer));
				std::transform(std::execution::seq, std::begin(layer), std::end(layer), std::begin(*err), std::begin(delta), [&] (auto & neuron, auto & e) {
					std::vector<double> delta = MeanSquared::weight_derivative(activation, e);
					std::transform(std::execution::seq, std::begin(neuron), std::end(neuron), std::begin(delta), std::begin(delta), [] (auto & weight, auto & delta) {
						return weight + delta;
					});
					return delta;
				});
				return delta;
			});
			
			++err;

#ifdef ANN_TMP_OPT
			std::for_each(std::execution::seq, std::begin(biases), std::prev(std::end(biases)), [&] (auto & neuron) {
#else
			// Calculate delta bias for this layer
			std::for_each(std::execution::seq, std::begin(biases), std::end(biases), [&] (auto & neuron) {
#endif
				std::transform(std::execution::seq, std::begin(neuron), std::end(neuron), std::begin(*--err), std::begin(neuron), [] (auto & bias, auto & error) {
					return bias + MeanSquared::bias_derivative(error);
				});
			});

#ifdef ANN_TMP_OPT
			_update(LogLikelihood(), act, err, out, weights, biases);
#endif
		}
		template <class Iterator1, class Iterator2, class Iterator3>
		void _update(LogLikelihood, Iterator1 & act, [[maybe_unused]] Iterator2 & err, Iterator3 & out, matrix<std::vector<double>> & weights, matrix<double> & biases)
		{
#ifdef ANN_TMP_OPT
			std::transform(std::execution::seq, std::prev(std::end(weights)), std::end(weights), act++, std::prev(std::end(weights)), [&] (auto & layer, auto & activation) {
#else
			// Calculate delta weight for this layer
			std::transform(std::execution::seq, std::begin(weights), std::end(weights), act++, std::begin(weights), [&] (auto & layer, auto & activation) {
#endif
				matrix<double> delta(std::size(layer));
				std::transform(std::execution::seq, std::begin(layer), std::end(layer), std::begin(*act), std::begin(delta), [&] (auto & neuron, auto & a) {
					std::vector<double> delta = LogLikelihood::weight_derivative(activation, a, *out);
					std::transform(std::execution::seq, std::begin(neuron), std::end(neuron), std::begin(delta), std::begin(delta), [] (auto & weight, auto & delta) {
						return weight + delta;
					});
					return delta;
				});
				return delta;
			});

#ifdef ANN_TMP_OPT
			std::for_each(std::execution::seq, std::prev(std::end(biases)), std::end(biases), [&] (auto & neuron) {
#else
			// Calculate delta bias for this layer
			std::for_each(std::execution::seq, std::begin(biases), std::end(biases), [&] (auto & neuron) {
#endif
				std::transform(std::execution::seq, std::begin(neuron), std::end(neuron), std::begin(neuron), std::begin(*act), [&] (auto & bias, auto & a) {
					return bias + LogLikelihood::bias_derivative(a, *out++);
				});
			});
		}

		std::vector<std::vector<Neuron>> network_matrix(std::initializer_list<size_t> const & layers)
		{
			std::vector<std::vector<Neuron>> matrix;
			matrix.reserve(std::size(layers));

			auto begin = std::next(std::begin(layers));
			matrix.emplace_back(*begin++, _input);

			std::for_each(begin, std::end(layers), [&matrix] (size_t const & layer) {
				matrix.emplace_back(layer, matrix.back());
			});

			return matrix;
		}

		double _eta;
		std::vector<double> _input;
		std::vector<std::vector<Neuron>> _network;
		std::vector<double_ref> _output;
	};
}

#endif
