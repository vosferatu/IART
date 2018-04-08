#ifndef NEURALNETWORK_HPP
#define NEURALNETWORK_HPP

#include <iostream>
#include <array>
#include <vector>
#include <numeric>
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
	***** Pointer Wrapper ******************************************************************************
	***************************************************************************************************/
	
	template <class Type>
	class pointer_wrapper
	{
	public:
		Type() = default;
		Type(Type * value) :
			_value(value)
		{
		}

		operator Type()
		{
			return *_value;
		}
	private:
		Type * _value;
	};

	using double_ptr = pointer_wrapper<double>;

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
			std::shared_lock<std::shared_mutex> lock(_lock);
			while (_counter.load(std::memory_order_relaxed) < _limit)
			{
				_trigger.wait(lock);
			}
		}
		void send()
		{
			if (_counter.fetch_add(1, std::memory_order_relaxed) + 1 == _limit)
			{
				std::scoped_lock<std::shared_mutex> lock(_lock);
				_trigger.notify_all();
			}
		}
		void clear()
		{
			_counter.store(0, std::memory_order_relaxed);
		}
	protected:
		std::shared_mutex _lock;
		std::condition_variable_any _trigger;
		std::atomic_size_t _counter;
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

		virtual void activation() = 0;

		NeuronCore & _receiver;
		NeuronCore & _sender;
	};

	class SigmoidNeuron : public Neuron
	{
	public:
		SigmoidNeuron(size_t inputs, NeuronCore & receiver, NeuronCore & sender) : Neuron(receiver, sender),
			_weights(inputs)
		{
		}
		SigmoidNeuron(SigmoidNeuron const & other) : Neuron(other._receiver, other._sender),
			_weights(other._weights.size()),
			_bias(GRG()),
			_inputs(other._inputs)
		{
			std::generate(std::begin(_weights), std::end(_weights), std::ref(GRG));
		}

		void ref_input(std::vector<double_ptr> inputs)
		{
			_inputs = inputs;
		}
		double & output_ref()
		{
			return _output;
		}

		void activation()
		{
			_receiver.receive();
			_output = 1.0 / (1.0 + std::exp(-std::inner_product(std::begin(_inputs), std::end(_inputs), std::begin(_weights), 0.0) - _bias));
			_sender.send();
		}
	private:
		std::vector<double> _weights;
		double _bias;
		std::vector<double_ptr> _inputs;
		double _output;
	};

	/***************************************************************************************************
	***** NeuralNetwork ********************************************************************************
	***************************************************************************************************/

	template <class NeuronType>
	class NeuralNetwork
	{
		static_assert(std::is_base_of_v<Neuron, NeuronType>, "NeuronType must be a subclass of Neuron");
	public:
		NeuralNetwork<NeuronType>(size_t inputs, std::initializer_list<size_t> layers) :
			_neurons(layers.size() + 1),
			_network(layers.size()),
			_inputs(inputs)
		{
			_neurons.at(0) = NeuronCore(1);
			std::transform(std::begin(layers), std::end(layers), std::next(std::begin(_neurons)), 
						   [] (size_t const & size) { return NeuronCore(size); });
			
			std::vector<double_ptr> inputs_ref(inputs);
			std::transform(std::begin(_inputs), std::end(_inputs), std::begin(inputs_ref),
						   [] (double & input) { return &input; });

			std::transform(std::begin(layers), std::end(layers), std::begin(_neurons), std::begin(_network),
						   [&inputs_ref] (size_t const & layer_size, NeuronCore & neuron_core)
			{
				NeuronType neuron(layer_size, neuron_core, *(&neuron_core + 1));
				neuron.ref_input(inputs_ref);

				std::vector<NeuronType> layer(layer_size, neuron);
				
				inputs_ref = std::vector<double_ptr>(layer_size);
				std::transform(std::begin(layer), std::end(layer), std::begin(inputs_ref),
							   [] (NeuronType & neuron) { return &(neuron.output_ref()); });

				return layer;
			});

			_outputs = inputs_ref;
		}

		// TODO
		// Training function
		// Testing function
	private:
		std::vector<NeuronCore> _neurons;
		std::vector<std::vector<NeuronType>> _network;
		std::vector<double> _inputs;
		std::vector<double_ptr> _outputs;
	};
}

#endif
