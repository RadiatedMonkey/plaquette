#pragma once

#include <plaquette/util.hpp>
#include <plaquette/instance.hpp>
#include <plaquette/device.hpp>
#include <plaquette/pipeline.hpp>
#include <plaquette/storage.hpp>

#include <volk.h>

#include <memory>

namespace Plaq {
    template<RealFloat F>
    struct Lattice {
        /// @brief The device-local buffer the lattice is stored in.
        ///
        /// The shader is given access to this buffer through a device address passed by push constant.
        std::shared_ptr<StorageBuffer> buffer; 
    };

    template<RealFloat F>
    struct SimulationConfig {
        PipelineConfig pipeline;
    };

    template<RealFloat F>
    class Simulation {
    public:
        Simulation(const SimulationConfig<F>& config) {
            mInstance = Instance::create();
            mDevice = mInstance->createDevice();
            mSimPipeline = mDevice->createPipeline(config.pipeline);
        }

        std::shared_ptr<Device>& device() {
            return mDevice;
        }

    protected:
        std::shared_ptr<Instance> mInstance = nullptr;
        std::shared_ptr<Device> mDevice = nullptr;
        std::shared_ptr<Pipeline> mSimPipeline = nullptr;
    };
}