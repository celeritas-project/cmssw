//----------------------------------*-C++-*----------------------------------//
// Copyright 2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file CeleritasSetup.cc
//---------------------------------------------------------------------------//
#include "SimG4Core/Application/interface/CeleritasSetup.hh"

#include <fstream>
#include <string>
#include <utility>
#include <G4GenericMessenger.hh>

#include "corecel/Assert.hh"
#include "corecel/sys/Device.hh"
#include "accel/AlongStepFactory.hh"
#include "celeritas/field/RZMapFieldInput.hh"

namespace celeritas
{//---------------------------------------------------------------------------//
/*!
 * Return non-owning pointer to a singleton.
 */
CeleritasSetup* CeleritasSetup::Instance()
{
    static CeleritasSetup setup;
    return &setup;
}

//---------------------------------------------------------------------------//
/*!
 * Set configurable properties from the UI.
 */
CeleritasSetup::CeleritasSetup()
{
    options_ = std::make_shared<SetupOptions>();
    field_ = G4ThreeVector(0, 0, 0);
    messenger_ = std::make_unique<G4GenericMessenger>(
        this, "/setup/", "celeritas geant integration setup");

    {
        auto& cmd
            = messenger_->DeclareProperty("rootBufferSize", root_buffer_size_);
        cmd.SetGuidance("Set the buffer size (bytes) of output root file");
        cmd.SetDefaultValue(std::to_string(root_buffer_size_));
    }
    {
        auto& cmd = messenger_->DeclareProperty("writeSDHits", write_sd_hits_);
        cmd.SetGuidance("Write a ROOT output file with hits from the SDs");
        cmd.SetDefaultValue("false");
    }
    {
        auto& cmd
	  = messenger_->DeclareProperty("skipMasterSD", skip_master_sd_);
        cmd.SetGuidance(
            "Emulate CMSSW by preventing SD construction on the master "
            "thread");
        cmd.SetDefaultValue("true");
    }
    {
        auto& cmd = messenger_->DeclareProperty("maxNumTracks",
                                                options_->max_num_tracks);
        cmd.SetGuidance("Set the maximum number of track slots");
        options_->max_num_tracks = Device::num_devices() > 0 ? 524288 : 64;
        cmd.SetDefaultValue(std::to_string(options_->max_num_tracks));
    }
    {
        auto& cmd = messenger_->DeclareProperty("maxNumEvents",
                                                options_->max_num_events);
        cmd.SetGuidance("Set the maximum number of events in the run");
        options_->max_num_events = 1024;
        cmd.SetDefaultValue(std::to_string(options_->max_num_events));
    }
    {
        auto& cmd = messenger_->DeclareProperty(
            "secondaryStackFactor", options_->secondary_stack_factor);
        cmd.SetGuidance("Set the number of secondary slots per track slot");
        options_->secondary_stack_factor = 3;
        cmd.SetDefaultValue(std::to_string(options_->secondary_stack_factor));
    }
    {
        auto& cmd = messenger_->DeclareProperty(
            "initializerCapacity", options_->initializer_capacity);
        cmd.SetGuidance("Set the maximum number of queued tracks");
        options_->initializer_capacity = 1048576;
        cmd.SetDefaultValue(std::to_string(options_->initializer_capacity));
    }
    {
        auto& cmd = messenger_->DeclareProperty("cudaStackSize",
                                                options_->cuda_stack_size);
        cmd.SetGuidance("Set the per-thread dynamic CUDA stack size (bytes)");
        options_->cuda_stack_size = 0;
    }
    {
        auto& cmd = messenger_->DeclareProperty("cudaHeapSize",
                                                options_->cuda_heap_size);
        cmd.SetGuidance("Set the shared dynamic CUDA heap size (bytes)");
        options_->cuda_heap_size = 0;
    }
    {
        messenger_->DeclareMethod("magFieldZ",
                                  &CeleritasSetup::SetMagFieldZTesla,
                                  "Set Z-axis magnetic field strength (T)");
    }
    {
        // TODO: expose other options here
    }

    // At setup time, get the field strength (native G4units)
    /*
    options_->make_along_step
        = UniformAlongStepFactory([this] { return field_; });
    */
    // Use a RZ map dumped from CMSSW
    options_->make_along_step
        = celeritas::RZMapFieldAlongStepFactory([] {
	      std::string filename = "cms-run3-rzfieldmap.json";
	      celeritas::RZMapFieldInput inp;
	      std::ifstream(filename) >> inp;
	      return inp; } );

    options_->get_num_streams = []{ return 4; };

    options_->output_file = "celeritas-output.json";

    // SensitiveDetector option
    if(write_sd_hits_)
    {
        celeritas::SDSetupOptions::StepPoint sp;

        sp.global_time = true;
        sp.position = true;
        sp.direction = true;
        sp.kinetic_energy = true;

        celeritas::SDSetupOptions sd;
        sd.pre = sp;
        sd.post = sp;
        sd.track = true;
        sd.locate_touchable = true;
        sd.enabled = true;

        options_->sd = sd;
    }
}

//---------------------------------------------------------------------------//
/*!
 * Set the list of ignored EM process names.
 */
void CeleritasSetup::SetIgnoreProcesses(SetupOptions::VecString ignored)
{
    options_->ignore_processes = std::move(ignored);
}

//---------------------------------------------------------------------------//
//! Default destructor
CeleritasSetup::~CeleritasSetup() = default;

//---------------------------------------------------------------------------//
/*!
 * Set the list of logical volumes with sensitive detectors.
 */
void CeleritasSetup::SetSDFromMaster(std::string filename)
{
    CELER_VALIDATE(std::ifstream(filename).good(),
                   << "Failed to open input file " << filename);

    std::ifstream input(filename);
    std::string line;
    std::unordered_set<std::string> vol_names;

    while (std::getline(input, line))
    {
        vol_names.insert(std::move(line));
    }
    options_->sd.force_volumes = celeritas::FindVolumes(vol_names);
}

//---------------------------------------------------------------------------//
}  // namespace celeritas
