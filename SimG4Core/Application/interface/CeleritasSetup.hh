//----------------------------------*-C++-*----------------------------------//
// Copyright 2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file CeleritasSetup.hh
//---------------------------------------------------------------------------//
#pragma once

#include <memory>
#include <fstream>
//#include <string>
#include <type_traits>
#include <CLHEP/Units/SystemOfUnits.h>
#include <G4ThreeVector.hh>

#include "accel/SetupOptions.hh"

class G4GenericMessenger;

namespace celeritas
{
//---------------------------------------------------------------------------//
/*!
 * Configuration for celeritas.
 */
class CeleritasSetup
{
  public:
    // Return non-owning pointer to a singleton
    static CeleritasSetup* Instance();

    //!@{
    //! \name setup options
    int GetRootBufferSize() const { return root_buffer_size_; }
    bool GetWriteSDHits() const { return write_sd_hits_; }
    bool GetSkipMasterSD() const { return skip_master_sd_; }
    //!@}

    //! Get a mutable reference to the setup options for DetectorConstruction
    SDSetupOptions& GetSDSetupOptions() { return options_->sd; }

    //! Get an immutable reference to the setup options
    std::shared_ptr<SetupOptions const> GetSetupOptions() const
    {
        return options_;
    }

    // Set the list of ignored EM process names
    void SetIgnoreProcesses(SetupOptions::VecString ignored);

    //! Set the field to this value (T) along the z axis
    void SetMagFieldZTesla(double f)
    {
        field_ = G4ThreeVector(0, 0, f * CLHEP::tesla);
    }

    //! Set sensitive detectors manually with a known list
    void SetSDFromMaster(std::string filename);

  private:
    // Private constructor since we're a singleton
    CeleritasSetup();
    ~CeleritasSetup();

    // Data
    std::shared_ptr<SetupOptions> options_;
    int root_buffer_size_{128000};
    bool write_sd_hits_{true};
    bool skip_master_sd_{true};
    G4ThreeVector field_;

    std::unique_ptr<G4GenericMessenger> messenger_;
};

//---------------------------------------------------------------------------//
}  // namespace celeritas
