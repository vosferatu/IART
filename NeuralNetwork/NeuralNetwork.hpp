#ifndef NEURALNETWORK_HPP
#define NEURALNETWORK_HPP

#include <iostream>
#include <array>
#include <vector>
#include <execution>
#include <optional>
#include <numeric>
#include <algorithm>
#include <atomic>
#include <random>
#include <shared_mutex>
#include <condition_variable>

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
	***** NeuronCore ***********************************************************************************
	***************************************************************************************************/

	class NeuronCore
	{
	public:
		NeuronCore() = default;
		NeuronCore(size_t limit) :
			_limit(limit)
		{
		}
		NeuronCore(NeuronCore const & other) :
			_limit(other._limit)
		{
		}
		NeuronCore & operator=(NeuronCore const & other)
		{
			_limit = other._limit;
			return *this;
		}
		
		void receive()
		{
			std::shared_lock<std::shared_mutex> lock(_mutex);
			while (_counter.load(std::memory_order_relaxed) < _limit)
			{
				_condition.wait(lock);
			}
		}
		void send()
		{
			if (_counter.fetch_add(1, std::memory_order_relaxed) + 1 == _limit)
			{
				std::scoped_lock<std::shared_mutex> lock(_mutex);
				_condition.notify_all();
			}
		}
		void clear(bool active)
		{
			_active.store(active, std::memory_order_relaxed);
			_counter.store(!active * _limit, std::memory_order_relaxed);

			if (!active)
			{
				std::scoped_lock<std::shared_mutex> lock(_mutex);
				_condition.notify_all();
			}
		}
		bool active()
		{
			return _active.load(std::memory_order_relaxed);
		}
	protected:
		std::atomic_bool _active{ true };
		std::shared_mutex _mutex;
		std::condition_variable_any _condition;
		std::atomic_size_t _counter{ 0 };
		size_t _limit;
	};

	/***************************************************************************************************
	***** Neuron ***************************************************************************************
	***************************************************************************************************/

	class Neuron
	{
	protected:
		Neuron(NeuronCore & receiver, NeuronCore & sender) :
			_receiver(receiver),
			_sender(sender)
		{
		}
		Neuron(Neuron const & other) :
			_receiver(other._receiver),
			_sender(other._sender)
		{
		}

		virtual void run() = 0;
		virtual void stop() = 0;
		virtual void _function() = 0;

		NeuronCore & _receiver;
		NeuronCore & _sender;
		std::thread _neuron;
	};

	class SigmoidNeuron : public Neuron
	{
		using double_ref = std::optional<std::reference_wrapper<double>>;
	public:
		using value_type = double;
		using reference_type = double_ref;

		SigmoidNeuron(size_t inputs, NeuronCore & receiver, NeuronCore & sender) : Neuron(receiver, sender),
			_weights(inputs),
			_bias(GRG()),
			_inputs(inputs),
			_output(0.5)
		{
			std::generate(std::begin(_weights), std::end(_weights), std::ref(GRG));
		}

		void ref_input(std::vector<double_ref> inputs)
		{
			_inputs = inputs;
		}
		double & output_ref()
		{
			return _output;
		}
		void run()
		{
			_neuron = std::move(std::thread(&SigmoidNeuron::_function, this));
		}
		void stop()
		{
			_neuron.join();
		}
	private:
		void _function() // Make this a public operator()
		{
			while (_receiver.active())
			{
				_receiver.receive();
				_output = 1.0 / (1.0 + std::exp(-std::inner_product(std::begin(_inputs), std::end(_inputs), std::begin(_weights), 0.0) - _bias));
				_sender.send();
			}
		}

		std::vector<double> _weights;
		double _bias;
		std::vector<double_ref> _inputs;
		double _output;
	};

	/***************************************************************************************************
	***** NeuralNetwork ********************************************************************************
	***************************************************************************************************/

	template <class NeuronType>
	class NeuralNetwork
	{
		static_assert(std::is_base_of_v<Neuron, NeuronType>, "NeuronType must derive from Neuron");
	public:
		NeuralNetwork<NeuronType>(size_t inputs, std::initializer_list<size_t> layers) :
			_neurons(layers.size() + 1),
			_network(layers.size()),
			_inputs(inputs)
		{
			_neurons.at(0) = NeuronCore(1);
			std::transform(std::begin(layers), std::end(layers), std::next(std::begin(_neurons)),
						   [] (size_t const & size) { return NeuronCore(size); });

			std::vector<typename NeuronType::reference_type> inputs_ref(inputs);
			std::transform(std::begin(_inputs), std::end(_inputs), std::begin(inputs_ref),
						   [] (typename NeuronType::value_type & input) { return &input; });

			std::transform(std::begin(layers), std::end(layers), std::begin(_neurons), std::begin(_network),
						   [&inputs_ref, &inputs] (size_t const & layer_size, NeuronCore & neuron_core)
			{
				NeuronType neuron(inputs, neuron_core, *(&neuron_core + 1));
				neuron.ref_input(inputs_ref);

				std::vector<NeuronType> layer(layer_size, neuron);

				inputs = layer_size;
				inputs_ref = std::vector<typename NeuronType::reference_type>(layer_size);
				std::transform(std::begin(layer), std::end(layer), std::begin(inputs_ref),
							   [] (NeuronType & neuron) { return &(neuron.output_ref()); });

				return layer;
			});

			_outputs = inputs_ref;

			std::for_each(std::begin(_network), std::end(_network), [] (std::vector<NeuronType> & neurons) 
			{ 
				std::for_each(std::begin(neurons), std::end(neurons), [] (NeuronType & neuron) 
				{
					neuron.run();
				}); 
			});
		}

		// TODO
		// Training function
		// Testing function

		~NeuralNetwork<NeuronType>()
		{
			std::transform(std::begin(_network), std::end(_network), std::begin(_neurons), std::begin(_neurons),
						   [] (std::vector<NeuronType> & neurons, NeuronCore & neuron_core)
			{
				neuron_core.clear(false);

				std::for_each(std::begin(neurons), std::end(neurons), [] (NeuronType & neuron)
				{
					neuron.stop();
				});

				return neuron_core; // self assignment is NOOP
			});
		}
	private:
		std::vector<NeuronCore> _neurons;
		std::vector<std::vector<NeuronType>> _network;
		std::vector<typename NeuronType::value_type> _inputs;
		std::vector<typename NeuronType::reference_type> _outputs;
	};
}

#endif
